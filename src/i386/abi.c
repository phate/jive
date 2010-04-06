#ifndef JIVE_I386_ABI_H
#define JIVE_I386_ABI_H

#include <jive/i386/machine.h>
#include <jive/instruction.h>
#include <jive/passthrough.h>
#include <jive/internal/subroutinestr.h>

typedef struct _jive_i386_subroutine {
	JIVE_SUBROUTINE_COMMON
	
	jive_passthrough * esp_passthrough, * return_value;
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

jive_subroutine *
jive_i386_subroutine_create(jive_graph * graph,
	size_t nparams, bool return_value)
{
	jive_i386_subroutine * sub = jive_malloc(graph, sizeof(*sub));
	sub->vmt = &jive_i386_subroutine_vmt;
	sub->stackframe = 0; /* FIXME */
	sub->graph = graph;
	
	jive_node * enter = jive_instruction_create(graph, &jive_pseudo_nop, 0, 0);
	jive_node * leave = jive_instruction_create(graph,
		&jive_i386_instructions[jive_i386_ret],
		0, 0);
	sub->enter = enter;
	sub->leave = leave;
	jive_state_edge_create(sub->enter, sub->leave);
	
	/* FIXME: should be grouped into a "callee-saved" helper */
	sub->esp_passthrough = jive_passthrough_create(graph, 32, &jive_i386_machine.regcls[jive_i386_gpr_esp], "stackptr");
	sub->stack_pointer = jive_output_passthrough_create(sub->enter, sub->esp_passthrough);
	sub->frame_pointer = sub->stack_pointer;
	jive_input_passthrough_create(sub->leave, sub->esp_passthrough, sub->stack_pointer);
	
	/* FIXME: must set esp the register non-spillable here */
	
	if (return_value)
		sub->return_value = jive_passthrough_create(graph, 32, &jive_i386_machine.regcls[jive_i386_gpr_eax], "return_value");
	else
		sub->return_value = 0;
	
	return (jive_subroutine *) sub;
}

#endif
