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
	jive::node * self,
	const jive::subroutine_op & op,
	jive_subroutine_stackframe_info * frame,
	const jive_subroutine_late_transforms * xfrm)
{
	jive::region * region = self->input(0)->origin()->region();
	return op.signature().abi_class->prepare_stackframe(
		op, region, frame, xfrm);
}

jive::input *
jive_subroutine_node_add_fp_dependency(
	const jive::node * self,
	const jive::subroutine_op & op,
	jive::node * node)
{
	jive::region * region = self->input(0)->origin()->region();
	return op.signature().abi_class->add_fp_dependency(
		op, region, node);
}

jive::input *
jive_subroutine_node_add_sp_dependency(
	const jive::node * self,
	const jive::subroutine_op & op,
	jive::node * node)
{
	jive::region * region = self->input(0)->origin()->region();
	return op.signature().abi_class->add_sp_dependency(
		op, region, node);
}

jive::node *
jive_region_get_subroutine_node(const jive::region * region)
{
	for (; region; region = region->parent()) {
		if (!region->anchor()) {
			continue;
		}
		jive::node * node = region->anchor()->node();
		if (dynamic_cast<const jive::subroutine_op *>(&node->operation())) {
			return node;
		}
	}
	return 0;
}

const struct jive_instructionset *
jive_region_get_instructionset(const jive::region * region)
{
	jive::node * sub = jive_region_get_subroutine_node(region);
	if (sub) {
		return static_cast<const jive::subroutine_op &>(sub->operation())
			.signature().abi_class->instructionset;
	} else {
		return NULL;
	}
}

jive::output *
jive_subroutine_node_get_sp(const jive::node * self)
{
	jive::region * region = self->input(0)->origin()->region();
	return static_cast<const jive::subroutine_op &>(self->operation())
		.get_passthrough_enter_by_index(region, 1);
}

jive::output *
jive_subroutine_node_get_fp(const jive::node * self)
{
	jive::region * region = self->input(0)->origin()->region();
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
	sub.region = new jive::region(graph->root(), graph);

	jive::node * enter = jive_opnode_create(jive::subroutine_head_op(), sub.region, {});

	for (size_t n = 0; n < sig.arguments.size(); ++n) {
		sub.builder_state->arguments[n].gate = jive_resource_class_create_gate(
			sig.arguments[n].rescls, graph, sig.arguments[n].name.c_str());
		sub.builder_state->arguments[n].output = enter->add_output(
			sub.builder_state->arguments[n].gate);
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
			sub.builder_state->passthroughs[n].gate = graph->create_gate(memory_type,
				sig.passthroughs[n].name.c_str());
		}
		sub.builder_state->passthroughs[n].gate->may_spill = sig.passthroughs[n].may_spill;
		sub.builder_state->passthroughs[n].output = enter->add_output(
			sub.builder_state->passthroughs[n].gate);
	}

	sub.signature = std::move(sig);

	return sub;
}

jive::node *
jive_subroutine_end(jive_subroutine & self)
{
	jive::output * control_return = self.hl_builder->finalize(self);
	jive::node * leave = jive_opnode_create(jive::subroutine_tail_op(), self.region,
		{self.region->top()->output(0), control_return});

	std::vector<jive::oport*> outputs;
	for (size_t n = 0; n < leave->noutputs(); n++)
		outputs.push_back(leave->output(n));

	jive::node * subroutine_node = jive_opnode_create(jive::subroutine_op(std::move(self.signature)),
		self.region->parent(), outputs);
	
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
