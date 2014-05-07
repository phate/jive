/*
 * Copyright 2010 2011 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/subroutine-private.h>
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

void
jive_subroutine_node_prepare_stackframe(
	jive_subroutine_node * self,
	const jive_subroutine_late_transforms * xfrm)
{
	return self->attrs.subroutine->abi_class->prepare_stackframe(self->attrs.subroutine, xfrm);
}

jive_input *
jive_subroutine_node_add_fp_dependency(const jive_subroutine_node * self, jive_node * node)
{
	return self->attrs.subroutine->abi_class->add_fp_dependency(self->attrs.subroutine, node);
}

jive_input *
jive_subroutine_node_add_sp_dependency(const jive_subroutine_node * self, jive_node * node)
{
	return self->attrs.subroutine->abi_class->add_sp_dependency(self->attrs.subroutine, node);
}

jive_subroutine_node *
jive_region_get_subroutine_node(const jive_region * region)
{
	for (; region; region = region->parent) {
		if (!region->anchor)
			continue;
		jive_subroutine_node * sub = jive_subroutine_node_cast(
			region->anchor->node);
		if (sub)
			return sub;
	}
	return 0;
}

const struct jive_instructionset *
jive_region_get_instructionset(const jive_region * region)
{
	jive_subroutine_node * sub = jive_region_get_subroutine_node(region);
	if (sub)
		return sub->attrs.subroutine->abi_class->instructionset;
	else
		return NULL;
}

jive_output *
jive_subroutine_node_get_sp(const jive_subroutine_node * self)
{
	return self->attrs.subroutine->passthroughs[1].output;
}

jive_output *
jive_subroutine_node_get_fp(const jive_subroutine_node * self)
{
	/* FIXME: this is only correct if we are compiling "omit-framepointer",
	but it is only a transitionary stage during subroutine refactoring */
	return self->attrs.subroutine->passthroughs[1].output;
}

jive_subroutine_stackframe_info *
jive_subroutine_node_get_stackframe_info(const jive_subroutine_node * self)
{
	return &self->attrs.subroutine->frame;
}

void
jive_subroutine_match_passthrough(
	const jive_subroutine_deprecated * old_subroutine,
	const jive_subroutine_passthrough * old_pt,
	jive_subroutine_deprecated * new_subroutine,
	jive_subroutine_passthrough * new_pt)
{
	new_pt->output = new_subroutine->enter->outputs[old_pt->output->index];
	new_pt->input = new_subroutine->leave->inputs[old_pt->input->index];
	new_pt->gate = new_pt->output->gate;
}

void
jive_subroutine_destroy(jive_subroutine_deprecated * self)
{
	if (self->subroutine_node)
		self->subroutine_node->attrs.subroutine = 0;
	self->class_->fini(self);
	jive_context_free(self->context, self);
}

void
jive_subroutine_create_region_and_nodes(jive_subroutine_deprecated * subroutine, jive_region * parent_region)
{
	jive_region * subroutine_region = jive_region_create_subregion(parent_region);
	subroutine_region->attrs.section = jive_region_section_code;
	jive_subroutine_leave_node_create(
		subroutine_region,
		jive_subroutine_enter_node_create(subroutine_region)->outputs[0]);
	jive_subroutine_node_create(subroutine_region, subroutine);
	subroutine->region = subroutine_region;
}

jive_subroutine_passthrough
jive_subroutine_create_passthrough(
	jive_subroutine_deprecated * subroutine,
	const jive_resource_class * cls,
	const char * name)
{
	jive_subroutine_passthrough passthrough;
	passthrough.gate = jive_resource_class_create_gate(
		cls, subroutine->subroutine_node->region->graph, name);
	passthrough.output = jive_node_gate_output(
		subroutine->enter, passthrough.gate);
	passthrough.input = jive_node_gate_input(
		subroutine->leave, passthrough.gate, passthrough.output);
	return passthrough;
}

jive_subroutine_passthrough
jive_subroutine_create_passthrough_memorystate(
	jive_subroutine_deprecated * subroutine,
	const char * name)
{
	jive_memory_type memory_type;
	
	jive_subroutine_passthrough passthrough;
	passthrough.gate = jive_type_create_gate(
		&memory_type, subroutine->subroutine_node->region->graph,
		name);
	passthrough.output = jive_node_gate_output(
		subroutine->enter, passthrough.gate);
	passthrough.input = jive_node_gate_input(
		subroutine->leave, passthrough.gate, passthrough.output);
	return passthrough;
}
	

jive_gate *
jive_subroutine_match_gate(jive_gate * gate, jive_node * old_node, jive_node * new_node)
{
	size_t n;
	
	JIVE_DEBUG_ASSERT(new_node->ninputs >= old_node->ninputs);
	for (n = 0; n < old_node->ninputs; n++) {
		if (old_node->inputs[n]->gate == gate)
			return new_node->inputs[n]->gate;
	}
	
	JIVE_DEBUG_ASSERT(new_node->noutputs >= old_node->noutputs);
	for (n = 0; n < old_node->noutputs; n++) {
		if (old_node->outputs[n]->gate == gate)
			return new_node->outputs[n]->gate;
	}
	
	return NULL;
}

jive_subroutine_deprecated *
jive_subroutine_copy(const jive_subroutine_deprecated * self,
	jive_node * new_enter_node, jive_node * new_leave_node)
{
	jive_graph * graph = new_enter_node->region->graph;
	jive_context * context = graph->context;
	
	jive_subroutine_deprecated * other = jive_context_malloc(context, sizeof(*other));
	jive_subroutine_init_(other, self->class_, context,
		self->nparameters, self->parameter_types,
		self->nreturns, self->return_types,
		self->npassthroughs);

	other->enter = (jive_subroutine_enter_node *) new_enter_node;
	other->leave = (jive_subroutine_leave_node *) new_leave_node;
	
	size_t n;
	
	for (n = 0; n < self->nparameters; n++) {
		jive_gate * old_gate = self->parameters[n];
		jive_gate * new_gate = NULL;
		
		if (!new_gate)
			new_gate = jive_subroutine_match_gate(old_gate, self->enter, new_enter_node);
		if (!new_gate)
			new_gate = jive_subroutine_match_gate(old_gate, self->leave, new_leave_node);
		if (!new_gate)
			new_gate = jive_resource_class_create_gate(old_gate->required_rescls, graph, old_gate->name);
		
		other->parameters[n] = new_gate;
	}
	
	for (n = 0; n < self->nreturns; n++) {
		jive_gate * old_gate = self->returns[n];
		jive_gate * new_gate = NULL;
		
		if (!new_gate)
			new_gate = jive_subroutine_match_gate(old_gate, self->enter, new_enter_node);
		if (!new_gate)
			new_gate = jive_subroutine_match_gate(old_gate, self->leave, new_leave_node);
		if (!new_gate)
			new_gate = jive_resource_class_create_gate(old_gate->required_rescls, graph, old_gate->name);
		
		other->returns[n] = new_gate;
	}
	
	for (n = 0; n < self->npassthroughs; ++n) {
		jive_subroutine_match_passthrough(
			self, &self->passthroughs[n],
			other, &other->passthroughs[n]);
	}
	
	return other;
}

jive_subroutine_deprecated *
jive_subroutine_create_takeover(
	jive_context * context, const jive_subroutine_class * class_,
	size_t nparameters, jive_gate * const parameters[],
	size_t nreturns, jive_gate * const returns[],
	size_t npassthroughs, const jive_subroutine_passthrough passthroughs[])
{
	/* FIXME: set parameter/return_types properly, add support in deserialization */
	jive_argument_type parameter_types[nparameters];
	jive_argument_type return_types[nreturns];

	jive_subroutine_deprecated * self = jive_context_malloc(context, sizeof(*self));
	jive_subroutine_init_(self, class_, context,
		nparameters, parameter_types, nreturns, return_types, npassthroughs);
	self->frame.upper_bound = 4;
	
	size_t n;
	
	for (n = 0; n < nparameters; n++)
		self->parameters[n] = parameters[n];
	
	for (n = 0; n < nreturns; n++)
		self->returns[n] = returns[n];
	
	for (n = 0; n < npassthroughs; n++) {
		self->passthroughs[n] = passthroughs[n];
	}

	return self;
}

void
jive_subroutine_init_(jive_subroutine_deprecated * self, const jive_subroutine_class * cls,
	jive_context * context,
	size_t nparameters, const jive_argument_type parameter_types[],
	size_t nreturns, const jive_argument_type return_types[],
	size_t npassthroughs)
{
	self->class_ = cls;
	
	self->context = context;
	
	self->enter = NULL;
	self->leave = NULL;
	self->subroutine_node = NULL;
	
	self->region = NULL;
	
	self->frame.lower_bound = 0;
	self->frame.upper_bound = 0;
	self->frame.frame_pointer_offset = 0;
	self->frame.stack_pointer_offset = 0;
	self->frame.call_area_size = 0;
	
	size_t n;
	
	self->nparameters = nparameters;
	self->parameters = jive_context_malloc(context, sizeof(self->parameters[0]) * nparameters);
	self->parameter_types = jive_context_malloc(context,
		sizeof(self->parameter_types[0]) * nparameters);
	for (n = 0; n < nparameters; n++) {
		self->parameters[n] = NULL;
		self->parameter_types[n] = parameter_types[n];
	}
	
	self->nreturns = nreturns;
	self->returns = jive_context_malloc(context, sizeof(self->returns[0]) * nreturns);
	self->return_types = jive_context_malloc(context, sizeof(self->returns[0]) * nreturns);
	for (n = 0; n < nreturns; n++) {
		self->returns[n] = NULL;
		self->return_types[n] = return_types[n];
	}
	
	self->npassthroughs = npassthroughs;
	self->passthroughs = jive_context_malloc(context, sizeof(self->passthroughs[0]) * npassthroughs);
	for (n = 0; n < npassthroughs; n++) {
		self->passthroughs[n].gate = NULL;
		self->passthroughs[n].input = NULL;
		self->passthroughs[n].output = NULL;
	}
}

void
jive_subroutine_fini_(jive_subroutine_deprecated * self)
{
	jive_context * context = self->context;
	jive_context_free(context, self->passthroughs);
	jive_context_free(context, self->parameters);
	jive_context_free(context, self->parameter_types);
	jive_context_free(context, self->returns);
	jive_context_free(context, self->return_types);
}

jive_node *
jive_subroutine_end(jive_subroutine self)
{
	self.region->bottom = self.old_subroutine_struct->leave;
	return self.old_subroutine_struct->subroutine_node;
}

jive_output *
jive_subroutine_simple_get_argument(
	jive_subroutine self,
	size_t index)
{
	return jive_subroutine_value_parameter(self.old_subroutine_struct, index);
}

void
jive_subroutine_simple_set_result(
	jive_subroutine self,
	size_t index,
	jive_output * value)
{
	jive_subroutine_value_return(self.old_subroutine_struct, index, value);
}

jive_output *
jive_subroutine_simple_get_global_state(const jive_subroutine self)
{
	return self.old_subroutine_struct->passthroughs[0].input->origin();
}

void
jive_subroutine_simple_set_global_state(jive_subroutine self, struct jive_output * state)
{
	self.old_subroutine_struct->passthroughs[0].input->divert_origin(state);
}
