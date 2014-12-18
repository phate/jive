/*
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/phi.h>

#include <stdio.h>

#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/phi-normal-form.h>
#include <jive/vsdg/substitution.h>
#include <jive/vsdg/valuetype.h>

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
	static const ctl::type type;
	return type;
}

jive_node *
phi_head_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return jive_opnode_create(*this, region, arguments, arguments + narguments);
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
	static const ctl::type type;
	return type;
}

jive_node *
phi_tail_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return jive_opnode_create(*this, region, arguments, arguments + narguments);
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

jive_node *
phi_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return jive_opnode_create(*this, region, arguments, arguments + narguments);
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
	jive_floating_region floating;
};

jive_phi
jive_phi_begin(jive_graph * graph)
{
	jive_phi self;
	jive_phi_build_state * state;
	state = new jive_phi_build_state;
	state->floating = jive_floating_region_create(graph);
	self.region = state->floating.region;
	
	jive::phi_head_op().create_node(self.region, 0, nullptr);
	
	self.internal_state = state;
	
	return self;
}

jive_phi_fixvar
jive_phi_fixvar_enter(jive_phi self, const struct jive::base::type * type)
{
	jive_phi_build_state * state = self.internal_state;
	jive_node * enter = self.region->top;
	jive_graph * graph = enter->region->graph;

	char gate_name[80];
	snprintf(gate_name, sizeof(gate_name), "fix_%p_%zd", enter, state->fixvars.size());
	
	jive_phi_fixvar fixvar;
	fixvar.gate = type->create_gate(graph, gate_name);
	fixvar.value = jive_node_gate_output(enter, fixvar.gate);
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
	jive_graph * graph = enter->region->graph;
	
	size_t n;
	
	jive_node * leave = jive::phi_tail_op().create_node(enter->region, 1, &enter->outputs[0]);
	for (n = 0; n < state->fixvars.size(); ++n)
		jive_node_gate_input(leave, state->fixvars[n].gate, state->fixvars[n].value);
	
	jive_floating_region_settle(state->floating);
	
	jive_node * anchor = jive::phi_op().create_node(self.region->parent, 1, &leave->outputs[0]);
	for (n = 0; n < state->fixvars.size(); ++n)
		state->fixvars[n].value = jive_node_gate_output(anchor, state->fixvars[n].gate);
	
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

jive_phi_extension *
jive_phi_begin_extension(jive_node * phi_node, size_t nfixvars,
	const jive::base::type * fixvar_types[])
{
	jive_graph * graph = phi_node->region->graph;
	jive_region * phi_region = jive_node_anchored_region(phi_node, 0);
	jive_node * enter = phi_region->top;

	jive_phi_extension * phi_ext = new jive_phi_extension;
	phi_ext->fixvars.resize(nfixvars);
	phi_ext->phi_node = phi_node;

	size_t n;
	char gate_name[80];
	size_t offset = enter->noutputs-1;
	for (n = 0; n < nfixvars; n++) {
		snprintf(gate_name, sizeof(gate_name), "fix_%p_%zd", enter, offset+n);
		jive::gate * gate = fixvar_types[n]->create_gate(graph, gate_name);
		phi_ext->fixvars[n] = jive_node_gate_output(enter, gate);
	}

	return phi_ext;
}

jive::output **
jive_phi_end_extension(struct jive_phi_extension * self)
{
	jive_node * phi_node = self->phi_node;
	jive_region * phi_region = jive_node_anchored_region(self->phi_node, 0);
	jive_node * enter = phi_region->top;
	jive_node * leave = phi_region->bottom;

	size_t n;
	size_t offset = leave->ninputs;
	for (n = 0; n < self->fixvars.size(); n++) {
		jive::gate * gate = enter->outputs[offset+n]->gate;
		jive_node_gate_input(leave, gate, self->fixvars[n]);
		jive_node_gate_output(phi_node, gate);
	}

	jive_graph_mark_denormalized(phi_node->region->graph);

	delete self;

	return &phi_node->outputs[offset-1];
}
