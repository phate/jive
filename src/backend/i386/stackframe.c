/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/instruction.h>
#include <jive/arch/stackframe-private.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/stackframe.h>
#include <jive/vsdg/cut.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>

static inline void
jive_i386_stackframe_init_(
	jive_i386_stackframe * self,
	jive_region * region,
	jive::output * stackptr_on_entry,
	jive::input * stackptr_on_exit)
{
	jive_stackframe_init_(&self->base, region, stackptr_on_entry);
	self->stackptr_on_entry = stackptr_on_entry;
	self->stackptr_on_exit = stackptr_on_exit;
	self->size = 0;
}

static void
jive_i386_stackframe_layout_(jive_stackframe * self_)
{
	jive_i386_stackframe * self = (jive_i386_stackframe *) self_;
	
	jive_stackvar_resource * var;
	JIVE_LIST_ITERATE(self->base.vars, var, stackframe_vars_list) {
		jive_stackslot * slot = var->slot;
		if (!var->slot) {
			self->size += 4;
			long offset = - self->size;
			slot = jive_stackslot_create(&self->base, offset);
			var->slot = slot;
		}
	}
	
	long stack_size = self->size;
	
	JIVE_LIST_ITERATE(self->base.vars, var, stackframe_vars_list) {
		jive_stackslot * slot = var->slot;
		
		jive::input * input;
		JIVE_LIST_ITERATE(var->base.inputs, input, resource_input_list) {
			const jive::instruction_op & op = static_cast<const jive::instruction_op &>(input->node->operation());
			if (op.icls() == &jive_i386_instr_int_load32_disp)
				op.immediates[0] = slot->offset + stack_size;
		}
		
		jive::output * output;
		JIVE_LIST_ITERATE(var->base.outputs, output, resource_output_list) {
			const jive::instruction_op & op = static_cast<const jive::instruction_op &>(input->node->operation());
			if (op.icls() == &jive_i386_instr_int_store32_disp)
				op.immediates[0] = slot->offset + stack_size;
		}
	}
	
	if (!self->size) return;
	
	/* move stack pointer */
	
	jive_region * region = self->region;
	
	jive::output * orig_stackptr = self->base.stackptr;
	jive_resource * stackptr_reg = orig_stackptr->resource;
	
	jive_node * stack_sub = (jive_node *) jive_instruction_node_create(
		region,
		&jive_i386_instr_int_sub_immediate,
		&orig_stackptr, &stack_size);
	
	jive_resource_assign_output(stackptr_reg, stack_sub->outputs[0]);
	jive_resource_assign_input(stackptr_reg, stack_sub->inputs[0]);
	jive_resource_assign_output(
		jive_output_get_constraint(stack_sub->outputs[1]),
		stack_sub->outputs[1]);
	
	jive::output * stackptr = stack_sub->outputs[0];
	
	jive::input * user, * next_user;
	JIVE_LIST_ITERATE_SAFE(orig_stackptr->users, user, next_user, output_users_list) {
		if (user->node != stack_sub) jive_input_divert_origin(user, stackptr);
		JIVE_DEBUG_ASSERT(user->resource);
	}
	
	jive_node * stack_add = (jive_node *) jive_instruction_node_create(
		region,
		&jive_i386_instr_int_add_immediate,
		&stackptr, &stack_size);
	
	jive_resource_assign_output(stackptr_reg, stack_add->outputs[0]);
	jive_resource_assign_input(stackptr_reg, stack_add->inputs[0]);
	jive_resource_assign_output(
		jive_output_get_constraint(stack_add->outputs[1]),
		stack_add->outputs[1]);
	
	jive::output * restored_stackptr = stack_add->outputs[0];
	
	jive_input_divert_origin(self->stackptr_on_exit, restored_stackptr);
	
	jive_cut_append(region->cuts.first, stack_sub);
	jive_cut_append(region->cuts.last->region_cuts_list.prev, stack_add);
	
	JIVE_DEBUG_ASSERT(self->stackptr_on_entry->resource);
	JIVE_DEBUG_ASSERT(self->stackptr_on_exit->resource);
}

const jive_stackframe_class JIVE_I386_STACKFRAME_CLASS = {
	fini : jive_stackframe_fini_,
	layout : jive_i386_stackframe_layout_
};

jive_stackframe *
jive_i386_stackframe_create(
	jive_region * region,
	jive::output * stackptr_on_entry,
	jive::input * stackptr_on_exit)
{
	jive_i386_stackframe * stackframe =
		jive_context_malloc(region->graph->context, sizeof(*stackframe));
	jive_i386_stackframe_init_(stackframe, region, stackptr_on_entry, stackptr_on_exit);
	stackframe->base.class_ = &JIVE_I386_STACKFRAME_CLASS;
	
	return &stackframe->base;
}
