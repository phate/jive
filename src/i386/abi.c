#ifndef JIVE_I386_ABI_H
#define JIVE_I386_ABI_H

#include <jive/i386/machine.h>
#include <jive/instruction.h>
#include <jive/passthrough.h>
#include <jive/internal/subroutinestr.h>

static jive_stackslot *
jive_i386_stackframe_allocate_slot(jive_stackframe * frame, jive_cpureg_class_t regcls)
{
	jive_stackslot * slot = jive_malloc(frame->graph, sizeof(*slot));
	
	slot->size = regcls->nbits/8;
	
	slot->prev = frame->last;
	slot->next = 0;
	if (frame->last) frame->last->next = slot;
	else frame->first =slot;
	frame->last = slot;
	
	return slot;
}

static const jive_stackframe_vmt jive_i386_stackframe_vmt = {
	.allocate_slot = &jive_i386_stackframe_allocate_slot,
	.finalize = 0
};

static jive_stackframe *
jive_i386_stackframe_create(jive_graph * graph, jive_value * stackpointer)
{
	jive_stackframe * frame = jive_malloc(graph, sizeof(*frame));
	frame->vmt = &jive_i386_stackframe_vmt;
	frame->graph = graph;
	frame->first = frame->last = 0;
	frame->stackpointer = stackpointer;
	
	return frame;
}

typedef struct _jive_i386_subroutine {
	JIVE_SUBROUTINE_COMMON
	
	jive_passthrough * esp_passthrough;
	jive_passthrough * ebp_passthrough;
	jive_passthrough * ebx_passthrough;
	jive_passthrough * edi_passthrough;
	jive_passthrough * esi_passthrough;
	jive_passthrough * return_value;
} jive_i386_subroutine;

static jive_value *
jive_i386_get_parameter(jive_subroutine * _sub, size_t index)
{
	jive_i386_subroutine * sub = (jive_i386_subroutine *) _sub;
	long offset = index * 4 + 4;
	
	jive_node * load = jive_instruction_create(sub->graph,
		&jive_i386_instructions[jive_i386_int_load32_disp],
		&sub->frame_pointer, &offset);
	
	return jive_instruction_output(load, 0);
}

static void
jive_i386_return_value(jive_subroutine * _sub, jive_value * value)
{
	jive_i386_subroutine * sub = (jive_i386_subroutine *) _sub;
	
	jive_input_passthrough_create(sub->leave, sub->return_value, value);
}

static const jive_subroutine_vmt jive_i386_subroutine_vmt = {
	.get_parameter = &jive_i386_get_parameter,
	.return_value = &jive_i386_return_value
};

static void
create_passthrough_reg(jive_i386_subroutine * sub, jive_graph * graph,
	jive_value ** value, jive_passthrough ** passthrough, jive_cpureg_class_t regcls, const char name[])
{
	*passthrough = jive_passthrough_create(graph, regcls->nbits, regcls, name);
	*value = jive_output_passthrough_create(sub->enter, *passthrough);
	jive_input_passthrough_create(sub->leave, *passthrough, *value);
}

jive_subroutine *
jive_i386_subroutine_create(jive_graph * graph,
	size_t nparams, bool return_value)
{
	jive_i386_subroutine * sub = jive_malloc(graph, sizeof(*sub));
	sub->vmt = &jive_i386_subroutine_vmt;
	sub->graph = graph;
	
	jive_node * enter = jive_instruction_create(graph, &jive_pseudo_nop, 0, 0);
	jive_node * leave = jive_instruction_create(graph,
		&jive_i386_instructions[jive_i386_ret],
		0, 0);
	sub->enter = enter;
	sub->leave = leave;
	jive_state_edge_create(sub->enter, sub->leave);
	
	/* FIXME: must set esp the register non-spillable here */
	create_passthrough_reg(sub, graph, &sub->stack_pointer, &sub->esp_passthrough, &jive_i386_machine.regcls[jive_i386_gpr_esp], "stackptr");
	sub->frame_pointer = sub->stack_pointer;
	
	jive_value * tmp;
	create_passthrough_reg(sub, graph, &tmp, &sub->ebp_passthrough, &jive_i386_machine.regcls[jive_i386_gpr_ebp], "callee_saved_ebp");
	create_passthrough_reg(sub, graph, &tmp, &sub->ebx_passthrough, &jive_i386_machine.regcls[jive_i386_gpr_ebx], "callee_saved_ebx");
	create_passthrough_reg(sub, graph, &tmp, &sub->esi_passthrough, &jive_i386_machine.regcls[jive_i386_gpr_esi], "callee_saved_esi");
	create_passthrough_reg(sub, graph, &tmp, &sub->edi_passthrough, &jive_i386_machine.regcls[jive_i386_gpr_edi], "callee_saved_edi");
	
	sub->stackframe = jive_i386_stackframe_create(graph, sub->stack_pointer);
	
	if (return_value)
		sub->return_value = jive_passthrough_create(graph, 32, &jive_i386_machine.regcls[jive_i386_gpr_eax], "return_value");
	else
		sub->return_value = 0;
	
	return (jive_subroutine *) sub;
}

#endif
