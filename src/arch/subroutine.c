/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/subroutine.h>

#include <string.h>

#include <jive/arch/memorytype.h>
#include <jive/arch/subroutine/nodes.h>
#include <jive/common.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/resource.h>

namespace jive {

subroutine_hl_builder_interface::~subroutine_hl_builder_interface() noexcept
{
}

}

void
jive_subroutine_node_prepare_stackframe(
	jive_node * self,
	const jive::subroutine_op & op,
	jive_subroutine_stackframe_info * frame,
	const jive_subroutine_late_transforms * xfrm)
{
	jive_region * region = self->inputs[0]->origin()->node()->region;
	return op.signature().abi_class->prepare_stackframe(
		op, region, frame, xfrm);
}

jive::input *
jive_subroutine_node_add_fp_dependency(
	const jive_node * self,
	const jive::subroutine_op & op,
	jive_node * node)
{
	jive_region * region = self->inputs[0]->origin()->node()->region;
	return op.signature().abi_class->add_fp_dependency(
		op, region, node);
}

jive::input *
jive_subroutine_node_add_sp_dependency(
	const jive_node * self,
	const jive::subroutine_op & op,
	jive_node * node)
{
	jive_region * region = self->inputs[0]->origin()->node()->region;
	return op.signature().abi_class->add_sp_dependency(
		op, region, node);
}

jive_node *
jive_region_get_subroutine_node(const jive_region * region)
{
	for (; region; region = region->parent) {
		if (!region->anchor) {
			continue;
		}
		jive_node * node = region->anchor->node();
		if (dynamic_cast<const jive::subroutine_op *>(&node->operation())) {
			return node;
		}
	}
	return 0;
}

const struct jive_instructionset *
jive_region_get_instructionset(const jive_region * region)
{
	jive_node * sub = jive_region_get_subroutine_node(region);
	if (sub) {
		return static_cast<const jive::subroutine_op &>(sub->operation())
			.signature().abi_class->instructionset;
	} else {
		return NULL;
	}
}

jive::output *
jive_subroutine_node_get_sp(const jive_node * self)
{
	jive_region * region = self->inputs[0]->origin()->node()->region;
	return static_cast<const jive::subroutine_op &>(self->operation())
		.get_passthrough_enter_by_index(region, 1);
}

jive::output *
jive_subroutine_node_get_fp(const jive_node * self)
{
	jive_region * region = self->inputs[0]->origin()->node()->region;
	/* FIXME: this is only correct if we are compiling "omit-framepointer",
	but it is only a transitionary stage during subroutine refactoring */
	return static_cast<const jive::subroutine_op &>(self->operation())
		.get_passthrough_enter_by_index(region, 1);
}

jive_subroutine
jive_subroutine_begin(
	jive_graph * graph,
	jive::subroutine_machine_signature sig,
	std::unique_ptr<jive::subroutine_hl_builder_interface> hl_builder)
{
	jive_subroutine sub;
	sub.hl_builder = std::move(hl_builder);
	sub.builder_state.reset(new jive::subroutine_builder_state(sig));
	sub.region = new jive_region(graph->root_region, graph);
	sub.region->attrs.section = jive_region_section_code;

	jive_node * enter = jive::subroutine_head_op().create_node(sub.region, 0, nullptr);

	for (size_t n = 0; n < sig.arguments.size(); ++n) {
		sub.builder_state->arguments[n].gate = jive_resource_class_create_gate(
			sig.arguments[n].rescls, graph, sig.arguments[n].name.c_str());
		sub.builder_state->arguments[n].output = jive_node_gate_output(
			enter, sub.builder_state->arguments[n].gate);
	}
	for (size_t n = 0; n < sig.results.size(); ++n) {
		sub.builder_state->results[n].gate = jive_resource_class_create_gate(
			sig.results[n].rescls, graph, sig.results[n].name.c_str());
	}
	for (size_t n = 0; n < sig.passthroughs.size(); ++n) {
		if (sig.passthroughs[n].rescls) {
			sub.builder_state->passthroughs[n].gate = jive_resource_class_create_gate(
				sig.passthroughs[n].rescls, graph, sig.passthroughs[n].name.c_str());
		} else {
			jive::mem::type memory_type;
			sub.builder_state->passthroughs[n].gate = jive_graph_create_gate(graph,
				sig.passthroughs[n].name.c_str(), memory_type);
		}
		sub.builder_state->passthroughs[n].gate->may_spill = sig.passthroughs[n].may_spill;
		sub.builder_state->passthroughs[n].output = jive_node_gate_output(
			enter, sub.builder_state->passthroughs[n].gate);
	}

	sub.signature = std::move(sig);

	return sub;
}

jive_node *
jive_subroutine_end(jive_subroutine & self)
{
	jive::output * control_return = self.hl_builder->finalize(self);
	jive::output * arguments[] = {self.region->top->outputs[0], control_return};
	jive_node * leave = jive::subroutine_tail_op().create_node(
		self.region, 2, arguments);

	jive_node * subroutine_node = jive::subroutine_op(std::move(self.signature)).create_node(
		self.region->parent, leave->noutputs, &leave->outputs[0]);
	
	for (const auto & pt : self.builder_state->passthroughs) {
		leave->add_input(pt.gate, pt.output);
	}
	for (const auto & res : self.builder_state->results) {
		leave->add_input(res.gate, res.output);
	}

	return subroutine_node;
}

jive::output *
jive_subroutine_simple_get_argument(
	jive_subroutine & self,
	size_t index)
{
	return self.hl_builder->value_parameter(self, index);
}

void
jive_subroutine_simple_set_result(
	jive_subroutine & self,
	size_t index,
	jive::output * value)
{
	self.hl_builder->value_return(self, index, value);
}

jive::output *
jive_subroutine_simple_get_global_state(const jive_subroutine & self)
{
	return self.builder_state->passthroughs[0].output;
}

void
jive_subroutine_simple_set_global_state(jive_subroutine & self, jive::output * state)
{
	self.builder_state->passthroughs[0].output = state;
}
