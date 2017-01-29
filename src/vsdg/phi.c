/*
 * Copyright 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/phi.h>

#include <stdio.h>

#include <jive/util/strfmt.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/phi-normal-form.h>
#include <jive/vsdg/seqtype.h>
#include <jive/vsdg/simple_node.h>
#include <jive/vsdg/substitution.h>

namespace jive {

phi_head_op::~phi_head_op() noexcept
{
}

size_t
phi_head_op::nresults() const noexcept
{
	return 1;
}

const base::type &
phi_head_op::result_type(size_t index) const noexcept
{
	return seq::seqtype;
}

std::string
phi_head_op::debug_string() const
{
	return "PHI_HEAD";
}

std::unique_ptr<jive::operation>
phi_head_op::copy() const
{
	return std::unique_ptr<jive::operation>(new phi_head_op(*this));
}

phi_tail_op::~phi_tail_op() noexcept
{
}

size_t
phi_tail_op::narguments() const noexcept
{
	return 1;
}

const base::type &
phi_tail_op::argument_type(size_t index) const noexcept
{
	return seq::seqtype;
}

std::string
phi_tail_op::debug_string() const
{
	return "PHI_TAIL";
}

std::unique_ptr<jive::operation>
phi_tail_op::copy() const
{
	return std::unique_ptr<jive::operation>(new phi_tail_op(*this));
}

phi_op::~phi_op() noexcept
{
}
std::string
phi_op::debug_string() const
{
	return "PHI";
}

std::unique_ptr<jive::operation>
phi_op::copy() const
{
	return std::unique_ptr<jive::operation>(new phi_op(*this));
}

}

static jive::node_normal_form *
jive_phi_node_get_default_normal_form_(
	const std::type_info & operator_class,
	jive::node_normal_form * parent_,
	jive_graph * graph)
{
	jive::phi_normal_form * nf = new jive::phi_normal_form(operator_class, parent_, graph);

	return nf;
}

static void  __attribute__((constructor))
register_node_normal_form(void)
{
	jive::node_normal_form::register_factory(
		typeid(jive::phi_op), jive_phi_node_get_default_normal_form_);
}


typedef struct jive_phi_build_state jive_phi_build_state;
struct jive_phi_build_state {
	jive::structural_node * node;
	std::vector<jive_phi_fixvar> fvs;
	std::unordered_map<jive::gate*, jive::oport*> fvmap;
};

jive_phi
jive_phi_begin(jive::region * parent)
{
	jive_phi self;
	auto state = new jive_phi_build_state;
	state->node = new jive::structural_node(jive::phi_op(), parent, 1);
	self.region = state->node->subregion(0);
	self.internal_state = state;

	return self;
}

jive_phi_fixvar
jive_phi_fixvar_enter(jive_phi self, const struct jive::base::type * type)
{
	auto state = self.internal_state;
	auto graph = self.region->graph();

	jive_phi_fixvar fixvar;
	auto str = jive::detail::strfmt("fix_", state->node, "_", state->fvs.size());
	fixvar.gate = graph->create_gate(*type, str);
	fixvar.value = self.region->add_argument(nullptr, fixvar.gate);

	state->fvs.push_back(fixvar);
	state->fvmap[fixvar.gate] = fixvar.value;

	return fixvar;
}

void
jive_phi_fixvar_leave(jive_phi self, jive::gate * var, jive::oport * fix_value)
{
	auto state = self.internal_state;

	if (state->fvmap.find(var) == state->fvmap.end())
		throw std::logic_error("Lookup of fix point variable failed");

	state->fvmap[var] = fix_value;
}

jive::node *
jive_phi_end(jive_phi self, size_t npost_values, jive_phi_fixvar * fix_values)
{
	auto state = self.internal_state;
	auto phi = state->node;

	std::unordered_map<jive::gate*, size_t> map;
	for (size_t n = 0; n < npost_values; n++)
		map[fix_values[n].gate] = n;

	for (auto & fv : state->fvs) {
		auto output = phi->add_output(fv.gate);
		self.region->add_result(state->fvmap[fv.gate], output, fv.gate);

		if (map.find(fv.gate) == map.end())
			throw std::logic_error("Lookup of fix point variable failed.");

		fix_values[map[fv.gate]].value = output;
	}

	delete state;

	return phi;
}
