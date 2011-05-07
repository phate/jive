#include <jive/backend/i386/subroutine.h>

#include <jive/backend/i386/registerset.h>
#include <jive/backend/i386/instructionset.h>

#include <jive/vsdg.h>
#include <jive/vsdg/function.h>
#include <jive/vsdg/substitution.h>


static inline void
jive_node_add_register_input(jive_node * node, const jive_register_class * regcls, jive_output * origin)
{
	const jive_type * type = jive_register_class_get_type(regcls);
	jive_input * retval = jive_node_add_input(node, type, origin);
	retval->required_rescls = &regcls->base;
}

static inline void
jive_function_region_callee_saved(jive_region * region, const jive_register_class * regcls)
{
	jive_gate * save_gate = jive_register_class_create_gate(regcls, region->graph, regcls->base.name);
	jive_node_gate_input(region->bottom, save_gate, jive_node_gate_output(region->top, save_gate));
}

/* convert according to "default" ABI */
jive_node *
jive_i386_subroutine_convert(jive_region * target_parent, jive_node * lambda_node)
{
	jive_context * context = target_parent->graph->context;
	jive_region * target_region = jive_function_region_create(target_parent);
	jive_region * src_region = lambda_node->inputs[0]->origin->node->region;
	
	JIVE_DECLARE_STATE_TYPE(memory_state_type);
	JIVE_DECLARE_CONTROL_TYPE(control_type);
	
	/* stackptr */
	jive_gate * stackptr_var = jive_register_class_create_gate(
		&jive_i386_regcls[jive_i386_gpr_esp], target_region->graph, "stackptr");
	jive_output * stackptr = jive_node_gate_output(target_region->top, stackptr_var);
	jive_node_gate_input(target_region->bottom, stackptr_var, stackptr);
	
	jive_substitution_map * subst = jive_substitution_map_create(context);
	
	/* collect & load all parameters */
	size_t n;
	for(n = 1; n < src_region->top->noutputs; n++) {
		jive_output * original = src_region->top->outputs[n];
		
		jive_output * stack_parameter = jive_node_add_output(target_region->top, memory_state_type);
		
		jive_node * load = jive_instruction_node_create(
			target_region,
			&jive_i386_instructions[jive_i386_int_load32_disp],
			(jive_output *[]){stackptr}, (long[]){n * 4});
		jive_node_add_input(load, memory_state_type, stack_parameter);
		
		jive_substitution_map_add_output(subst, original, load->outputs[0]);
	}
	
	/* transfer function body */
	jive_region_copy_substitute(src_region, target_region, subst, false, false);
	
	/* substitute back return value */
	if (src_region->bottom->ninputs > 1) {
		jive_output * retval = jive_substitution_map_lookup_output(subst, src_region->bottom->inputs[1]->origin);
		jive_node_add_register_input(target_region->bottom, &jive_i386_regcls[jive_i386_gpr_eax], retval);
	}
	
	jive_substitution_map_destroy(subst);
	
	/* add callee-saved registers */
	jive_function_region_callee_saved(target_region, &jive_i386_regcls[jive_i386_gpr_ebx]);
	jive_function_region_callee_saved(target_region, &jive_i386_regcls[jive_i386_gpr_ebp]);
	jive_function_region_callee_saved(target_region, &jive_i386_regcls[jive_i386_gpr_esi]);
	jive_function_region_callee_saved(target_region, &jive_i386_regcls[jive_i386_gpr_edi]);
	
	jive_node * ret_instr = jive_instruction_node_create(target_region, &jive_i386_instructions[jive_i386_ret], NULL, NULL);
	/* depends on return address on stack */
	jive_node_add_input(ret_instr, memory_state_type, jive_node_add_output(target_region->top, memory_state_type));
	/* produces control output */
	jive_input_divert_origin(target_region->bottom->inputs[0], jive_node_add_output(ret_instr, control_type));
	
	return jive_lambda_node_create(target_region);
}
