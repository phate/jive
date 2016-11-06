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
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/phi-normal-form.h>
#include <jive/vsdg/seqtype.h>
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
	std::vector<jive_phi_fixvar> fixvars;
};

jive_phi
jive_phi_begin(jive_region * parent)
{
	jive_phi self;
	jive_phi_build_state * state;
	state = new jive_phi_build_state;
	self.region = new jive_region(parent, parent->graph());

	jive::phi_head_op().create_node(self.region, {});
	
	self.internal_state = state;
	
	return self;
}

jive_phi_fixvar
jive_phi_fixvar_enter(jive_phi self, const struct jive::base::type * type)
{
	jive_phi_build_state * state = self.internal_state;
	jive_node * enter = self.region->top;
	jive_graph * graph = enter->graph();

	jive_phi_fixvar fixvar;
	fixvar.gate = jive_graph_create_gate(
		graph,
		jive::detail::strfmt("fix_", enter, "_", state->fixvars.size()),
		*type);
	fixvar.value = enter->add_output(fixvar.gate);
	state->fixvars.push_back(fixvar);

	return fixvar;
}

void
jive_phi_fixvar_leave(jive_phi self, jive::gate * var, jive::output * fix_value)
{
	jive_phi_build_state * state = self.internal_state;
	size_t n;
	for (n = 0; n < state->fixvars.size(); ++n) {
		if (state->fixvars[n].gate != var)
			continue;
		state->fixvars[n].value = fix_value;
		return;
	}
	
	throw std::logic_error("Lookup of fix point variable failed");
}

jive_node *
jive_phi_end(jive_phi self,
	size_t npost_values, jive_phi_fixvar * fix_values)
{
	jive_phi_build_state * state = self.internal_state;
	jive_node * enter = self.region->top;

	size_t n;
	jive::output * tmp = enter->output(0);
	jive_node * leave = jive::phi_tail_op().create_node(enter->region(), {tmp});
	for (n = 0; n < state->fixvars.size(); ++n)
		leave->add_input(state->fixvars[n].gate, state->fixvars[n].value);

	tmp = leave->output(0);
	jive_node * anchor = jive::phi_op().create_node(self.region->parent(), {tmp});
	for (n = 0; n < state->fixvars.size(); ++n)
		state->fixvars[n].value = anchor->add_output(state->fixvars[n].gate);
	
	for (n = 0; n < npost_values; ++n) {
		size_t k;
		for (k = 0; k < state->fixvars.size(); ++k) {
			if (state->fixvars[k].gate == fix_values[n].gate) {
				fix_values[n].value = state->fixvars[k].value;
				break;
			}
		}
		if (k == state->fixvars.size()) {
			throw std::logic_error("Lookup of fix point variable failed");
		}
	}
	
	delete state;
	
	return anchor;
}
