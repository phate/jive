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
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/seqtype.h>

namespace jive {

theta_head_op::~theta_head_op() noexcept
{
}

size_t
theta_head_op::nresults() const noexcept
{
	return 1;
}

const base::type &
theta_head_op::result_type(size_t index) const noexcept
{
	return seq::seqtype;
}

std::string
theta_head_op::debug_string() const
{
	return "THETA_HEAD";
}

std::unique_ptr<jive::operation>
theta_head_op::copy() const
{
	return std::unique_ptr<jive::operation>(new theta_head_op(*this));
}

theta_tail_op::~theta_tail_op() noexcept
{
}

size_t
theta_tail_op::narguments() const noexcept
{
	return 2;
}

const base::type &
theta_tail_op::argument_type(size_t index) const noexcept
{
	if (index == 0)
		return seq::seqtype;
	else {
		return jive::ctl::boolean;
	}
}

std::string
theta_tail_op::debug_string() const
{
	return "THETA_TAIL";
}

std::unique_ptr<jive::operation>
theta_tail_op::copy() const
{
	return std::unique_ptr<jive::operation>(new theta_tail_op(*this));
}

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
	std::vector<jive_theta_loopvar> loopvars;
};

jive_theta
jive_theta_begin(jive_region * parent)
{
	jive_theta self;
	jive_theta_build_state * state = new jive_theta_build_state;
	self.region = new jive_region(parent, parent->graph);

	self.region->attrs.is_looped = true;
	jive::theta_head_op().create_node(self.region, {});
	
	self.internal_state = state;
	
	return self;
}

jive_theta_loopvar
jive_theta_loopvar_enter(jive_theta self, jive::output * pre_value)
{
	jive_theta_build_state * state = self.internal_state;
	jive_node * head = self.region->top;
	jive_graph * graph = head->graph();
	
	size_t index = state->loopvars.size();
	
	const jive::base::type * type = &pre_value->type();
	state->loopvars.resize(state->loopvars.size()+1);
	
	state->loopvars[index].gate = jive_graph_create_gate(
		graph, jive::detail::strfmt("loopvar_", head, "_", index), *type);
	head->add_input(state->loopvars[index].gate, pre_value);
	state->loopvars[index].value = head->add_output(state->loopvars[index].gate);
	
	return state->loopvars[index];
}

void
jive_theta_loopvar_leave(jive_theta self, jive::gate * var, jive::output * post_value)
{
	jive_theta_build_state * state = self.internal_state;
	size_t n;
	for (n = 0; n < state->loopvars.size(); ++n) {
		if (state->loopvars[n].gate != var)
			continue;
		state->loopvars[n].value = post_value;
		return;
	}
	
	throw std::logic_error("Lookup of loop-variant variable failed");
}

jive_node *
jive_theta_end(jive_theta self, jive::output * predicate,
	size_t npost_values, jive_theta_loopvar * post_values)
{
	jive_theta_build_state * state = self.internal_state;

	size_t n;
	jive_node * tail = jive::theta_tail_op().create_node(self.region,
		{self.region->top->output(0), predicate});
	for (n = 0; n < state->loopvars.size(); ++n)
		tail->add_input(state->loopvars[n].gate, state->loopvars[n].value);

	jive::output * tmp = tail->output(0);
	jive_node * anchor = jive::theta_op().create_node(self.region->parent, {tmp});
	for (n = 0; n < state->loopvars.size(); ++n)
		state->loopvars[n].value = anchor->add_output(state->loopvars[n].gate);
	
	for (n = 0; n < npost_values; ++n) {
		size_t k;
		for (k = 0; k < state->loopvars.size(); ++k) {
			if (state->loopvars[k].gate == post_values[n].gate) {
				post_values[n].value = state->loopvars[k].value;
				break;
			}
		}
	}
	
	delete state;
	
	return anchor;
}

