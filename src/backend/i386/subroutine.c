#include <jive/backend/i386/subroutine-private.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/registerset.h>
#include <jive/backend/i386/stackframe.h>
#include <jive/arch/instruction.h>
#include <jive/vsdg/region.h>

static void
_jive_i386_subroutine_fini(jive_subroutine * self_)
{
	jive_i386_subroutine * self = (jive_i386_subroutine *) self_;
	jive_context * context = self->base.enter->graph->context;
	jive_context_free(context, self->arguments);
}

static jive_output *
_jive_i386_subroutine_get_parameter(jive_subroutine * self_, size_t index)
{
	jive_i386_subroutine * self = (jive_i386_subroutine *) self_;
	
	/* FIXME: use stackslot, and don't load multiple times */
	jive_node * load = (jive_node *) jive_instruction_node_create(
		self->base.enter->region,
		&jive_i386_instructions[jive_i386_int_load32_disp],
		(jive_output *[]){self->stackframe->stackptr}, (long[]){4 + index * 4});
	
	return load->outputs[0];
}

static jive_input *
_jive_i386_subroutine_return_value(jive_subroutine * self_, jive_output * value)
{
	jive_i386_subroutine * self = (jive_i386_subroutine *) self_;
	return jive_node_gate_input(self->base.leave, self->return_variable, value);
}

const jive_subroutine_class JIVE_I386_SUBROUTINE_CLASS = {
	.fini = _jive_i386_subroutine_fini,
	.get_parameter = _jive_i386_subroutine_get_parameter,
	.return_value = _jive_i386_subroutine_return_value
};

jive_subroutine *
jive_i386_subroutine_create(
	jive_region * region,
	size_t narguments, const jive_argument_type arguments[],
	jive_argument_type return_type)
{
	jive_graph * graph = region->graph;
	jive_context * context = graph->context;
	jive_i386_subroutine * subroutine = jive_context_malloc(context, sizeof(*subroutine));
	
	jive_region * subregion = jive_region_create_subregion(region);
	
	jive_node * enter = (jive_node *) jive_instruction_node_create(
		subregion,
		&JIVE_PSEUDO_NOP,
		NULL, NULL);
	
	jive_node * leave = (jive_node *) jive_instruction_node_create(
		subregion,
		&jive_i386_instructions[jive_i386_ret],
		NULL, NULL);
	
	subroutine->base.class_ = &JIVE_I386_SUBROUTINE_CLASS;
	_jive_subroutine_init(&subroutine->base, enter, leave);
	
	jive_gate * stackptr_var = jive_regcls_create_gate(
		&jive_i386_regcls[jive_i386_gpr_esp], graph, "stackptr");
	subroutine->return_variable = jive_regcls_create_gate(
		&jive_i386_regcls[jive_i386_gpr_eax], graph, "retval");
	
	jive_gate * save_ebx = jive_regcls_create_gate(
		&jive_i386_regcls[jive_i386_gpr_ebx], graph, "save_ebx");
	jive_gate * save_ebp = jive_regcls_create_gate(
		&jive_i386_regcls[jive_i386_gpr_ebp], graph, "save_ebp");
	jive_gate * save_edi = jive_regcls_create_gate(
		&jive_i386_regcls[jive_i386_gpr_edi], graph, "save_edi");
	jive_gate * save_esi = jive_regcls_create_gate(
		&jive_i386_regcls[jive_i386_gpr_esi], graph, "save_esi");
	
	jive_output * stackptr = jive_node_gate_output(enter, stackptr_var);
	jive_node_gate_input(leave, stackptr_var, stackptr);
	jive_node_gate_input(leave, save_ebx, jive_node_gate_output(enter, save_ebx));
	jive_node_gate_input(leave, save_ebp, jive_node_gate_output(enter, save_ebp));
	jive_node_gate_input(leave, save_esi, jive_node_gate_output(enter, save_esi));
	jive_node_gate_input(leave, save_edi, jive_node_gate_output(enter, save_edi));
	
	subroutine->stackframe = jive_i386_stackframe_create(subregion, stackptr);
	
	subroutine->arguments = jive_context_malloc(context, sizeof(subroutine->arguments[0]) * narguments);
	subroutine->narguments = narguments;
	size_t n;
	for(n=0; n<narguments; n++) subroutine->arguments[n] = arguments[n];
	subroutine->return_type = return_type;
	
	return &subroutine->base;
}
