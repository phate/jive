/*
 * Copyright 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/theta.h>

#include <stdio.h>
#include <string.h>

#include <jive/common.h>
#include <jive/util/strfmt.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/seqtype.h>
#include <jive/vsdg/simple_node.h>
#include <jive/vsdg/structural_node.h>

namespace jive {

theta_op::~theta_op() noexcept
{
}
std::string
theta_op::debug_string() const
{
	return "THETA";
}

std::unique_ptr<jive::operation>
theta_op::copy() const
{
	return std::unique_ptr<jive::operation>(new theta_op(*this));
}

}

typedef struct jive_theta_build_state jive_theta_build_state;
struct jive_theta_build_state {
	jive::structural_node * node;
	std::vector<jive_theta_loopvar> lvs;
	std::unordered_map<jive::gate*, jive::oport*> lvmap;
};

jive_theta
jive_theta_begin(jive::region * parent)
{
	jive_theta self;
	auto state = new jive_theta_build_state;
	state->node = parent->add_structural_node(jive::theta_op(), 1);
	self.region = state->node->subregion(0);
	self.internal_state = state;
	
	return self;
}

jive_theta_loopvar
jive_theta_loopvar_enter(jive_theta self, jive::oport * pre_value)
{
	auto state = self.internal_state;
	auto graph = self.region->graph();

	jive_theta_loopvar lv;
	auto str = jive::detail::strfmt("loopvar_", state->node, "_", state->lvs.size());
	lv.gate = graph->create_gate(pre_value->type(), str);
	auto input = state->node->add_input(lv.gate, pre_value);
	lv.value = self.region->add_argument(input, lv.gate);

	state->lvs.push_back(lv);
	state->lvmap[lv.gate] = lv.value;

	return lv;
}

void
jive_theta_loopvar_leave(jive_theta self, jive::gate * var, jive::oport * post_value)
{
	auto state = self.internal_state;

	if (state->lvmap.find(var) == state->lvmap.end())
		throw std::logic_error("Lookup of loop-variant variable failed.");

	state->lvmap[var] = post_value;
}

jive::node *
jive_theta_end(jive_theta self, jive::oport * predicate,
	size_t npost_values, jive_theta_loopvar * post_values)
{
	auto state = self.internal_state;
	auto theta = state->node;

	std::unordered_map<jive::gate*, size_t> map;
	for (size_t n = 0; n < npost_values; n++)
		map[post_values[n].gate] = n;

	self.region->add_result(predicate, nullptr, predicate->type());
	for (auto & lv : state->lvs) {
		auto output = theta->add_output(lv.gate);
		self.region->add_result(state->lvmap[lv.gate], output, lv.gate);
		lv.value = output;

		if (map.find(lv.gate) == map.end())
			throw std::logic_error("Lookup of fix loop-variant variable failed.");

		post_values[map[lv.gate]].value = output;
	}

	delete state;

	return theta;
}
