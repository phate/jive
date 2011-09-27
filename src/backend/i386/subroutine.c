#include <jive/backend/i386/subroutine.h>

#include <stdio.h>

#include <jive/arch/stackslot.h>
#include <jive/arch/subroutine-private.h>
#include <jive/backend/i386/registerset.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/regalloc/auxnodes.h>
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
jive_subroutine *
jive_i386_subroutine_convert(jive_region * target_parent, jive_node * lambda_node)
{
	jive_region * src_region = lambda_node->inputs[0]->origin->node->region;
	jive_context * context = target_parent->graph->context;
	
	size_t nparameters = src_region->top->noutputs - 1;
	size_t nreturns = src_region->bottom->ninputs - 1;
	
	jive_argument_type parameters[nparameters];
	jive_argument_type returns[nreturns];
	
	size_t n;
	for (n = 0; n < nparameters; n++)
		parameters[n] = jive_argument_long; /* FIXME: pick correct type */
	for (n = 0; n < nreturns; n++)
		returns[n] = jive_argument_long; /* FIXME: pick correct type */
	
	jive_subroutine * subroutine = jive_i386_subroutine_create(target_parent,
		nparameters, parameters,
		nreturns, returns);

	jive_substitution_map * subst = jive_substitution_map_create(context);
	
	/* map all parameters */
	for (n = 1; n < src_region->top->noutputs; n++) {
		jive_output * original = src_region->top->outputs[n];
		
		jive_output * substitute = jive_subroutine_value_parameter(subroutine, n - 1);
		
		jive_substitution_map_add_output(subst, original, substitute);
	}
	
	/* transfer function body */
	jive_region_copy_substitute(src_region, subroutine->region, subst, false, false);
	
	for (n = 1; n < src_region->bottom->ninputs; n++) {
		jive_output * retval = jive_substitution_map_lookup_output(subst, src_region->bottom->inputs[1]->origin);
		jive_subroutine_value_return(subroutine, n - 1, retval);
	}
	
	jive_substitution_map_destroy(subst);
	
	return subroutine;
}

static jive_i386_subroutine *
jive_i386_subroutine_alloc(jive_region * region, size_t nparameters, size_t nreturns);

static const jive_subroutine_class JIVE_I386_SUBROUTINE;

static jive_i386_subroutine *
jive_i386_subroutine_alloc(jive_region * region, size_t nparameters, size_t nreturns);

static void
jive_i386_subroutine_fini_(jive_subroutine * self_);

static jive_output *
jive_i386_subroutine_value_parameter_(jive_subroutine * self_, size_t index);

static jive_input *
jive_i386_subroutine_value_return_(jive_subroutine * self_, size_t index, jive_output * value);

static jive_subroutine *
jive_i386_subroutine_copy_(const jive_subroutine * self_,
	jive_node * new_enter_node, jive_node * new_leave_node);

static const jive_subroutine_class JIVE_I386_SUBROUTINE = {
	.fini = jive_i386_subroutine_fini_,
	.value_parameter = jive_i386_subroutine_value_parameter_,
	.value_return = jive_i386_subroutine_value_return_,
	.copy = jive_i386_subroutine_copy_
};

static jive_i386_subroutine *
jive_i386_subroutine_alloc(jive_region * region, size_t nparameters, size_t nreturns)
{
	jive_graph * graph = region->graph;
	jive_context * context = graph->context;
	
	jive_i386_subroutine * self;
	self = jive_context_malloc(context, sizeof(*self));
	jive_subroutine_init(&self->base, &JIVE_I386_SUBROUTINE, context);
	
	size_t n;
	
	self->base.nparameters = nparameters;
	self->base.parameters = jive_context_malloc(context, sizeof(self->base.parameters[0]) * nparameters);
	
	for (n = 0; n < nparameters; n++)
		self->base.parameters[n] = NULL;
	
	self->base.nreturns = nreturns;
	self->base.returns = jive_context_malloc(context, sizeof(self->base.returns[0]) * nreturns);
	
	for (n = 0; n < nreturns; n++)
		self->base.returns[n] = NULL;
	
	jive_subroutine_init(&self->base, &JIVE_I386_SUBROUTINE, context);
	
	return self;
}

static void
jive_i386_subroutine_fini_(jive_subroutine * self_)
{
	jive_i386_subroutine * self = (jive_i386_subroutine *) self_;
	jive_context_free(self->base.context, self->base.parameters);
	jive_context_free(self->base.context, self->base.returns);
}

static jive_output *
jive_i386_subroutine_value_parameter_(jive_subroutine * self_, size_t index)
{
	jive_i386_subroutine * self = (jive_i386_subroutine *) self_;
	jive_gate * gate = self->base.parameters[index];
	jive_output * output = jive_node_gate_output(&self->base.enter->base, gate);
	
	const jive_type * in_type = jive_gate_get_type(gate);
	const jive_type * out_type = jive_resource_class_get_type(&jive_i386_regcls[jive_i386_gpr].base);
	jive_node * node = jive_aux_split_node_create(self->base.enter->base.region,
		in_type, output, gate->required_rescls,
		out_type, &jive_i386_regcls[jive_i386_gpr].base);
	output = node->outputs[0];
	
	return output;
}

static jive_input *
jive_i386_subroutine_value_return_(jive_subroutine * self_, size_t index, jive_output * value)
{
	jive_i386_subroutine * self = (jive_i386_subroutine *) self_;
	jive_gate * gate = self->base.returns[index];
	return jive_node_gate_input(&self->base.leave->base, gate, value);
}

static jive_subroutine *
jive_i386_subroutine_copy_(const jive_subroutine * self_,
	jive_node * new_enter_node, jive_node * new_leave_node)
{
	jive_graph * graph = new_enter_node->region->graph;
	jive_i386_subroutine * self = (jive_i386_subroutine *) self_;
	
	jive_i386_subroutine * other = jive_i386_subroutine_alloc(new_enter_node->region, self->base.nparameters, self->base.nreturns);
	other->base.enter = (jive_subroutine_enter_node *) new_enter_node;
	other->base.leave = (jive_subroutine_leave_node *) new_leave_node;
	
	size_t n;
	
	for (n = 0; n < self->base.nparameters; n++) {
		jive_gate * old_gate = self->base.parameters[n];
		jive_gate * new_gate = NULL;
		
		if (!new_gate)
			new_gate = jive_subroutine_match_gate(old_gate, &self->base.enter->base, new_enter_node);
		if (!new_gate)
			new_gate = jive_subroutine_match_gate(old_gate, &self->base.leave->base, new_leave_node);
		if (!new_gate)
			new_gate = jive_resource_class_create_gate(old_gate->required_rescls, graph, old_gate->name);
		
		other->base.parameters[n] = new_gate;
	}
	
	for (n = 0; n < self->base.nreturns; n++) {
		jive_gate * old_gate = self->base.returns[n];
		jive_gate * new_gate = NULL;
		
		if (!new_gate)
			new_gate = jive_subroutine_match_gate(old_gate, &self->base.enter->base, new_enter_node);
		if (!new_gate)
			new_gate = jive_subroutine_match_gate(old_gate, &self->base.leave->base, new_leave_node);
		if (!new_gate)
			new_gate = jive_resource_class_create_gate(old_gate->required_rescls, graph, old_gate->name);
		
		other->base.returns[n] = new_gate;
	}
	
	jive_subroutine_match_passthrough(&self->base, &self->saved_esp, &other->base, &other->saved_esp);
	jive_subroutine_match_passthrough(&self->base, &self->saved_ebx, &other->base, &other->saved_ebx);
	jive_subroutine_match_passthrough(&self->base, &self->saved_ebp, &other->base, &other->saved_ebp);
	jive_subroutine_match_passthrough(&self->base, &self->saved_esi, &other->base, &other->saved_esi);
	jive_subroutine_match_passthrough(&self->base, &self->saved_edi, &other->base, &other->saved_edi);
	
	return &other->base;
}

jive_subroutine *
jive_i386_subroutine_create(jive_region * region,
	size_t nparameters, const jive_argument_type parameters[],
	size_t nreturns, const jive_argument_type returns[])
{
	jive_graph * graph = region->graph;
	jive_i386_subroutine * self = jive_i386_subroutine_alloc(region, nparameters, nreturns);
	self->base.frame.upper_bound = 4;
	
	size_t n;
	
	for (n = 0; n < nparameters; n++) {
		char argname[80];
		snprintf(argname, sizeof(argname), "arg%zd", n + 1);
		const jive_resource_class * cls;
		cls = jive_fixed_stackslot_class_get(4, 4, (n + 1) * 4);
		self->base.parameters[n] = jive_resource_class_create_gate(cls, graph, argname);
		self->base.frame.upper_bound = (n + 2) * 4;
	}
	
	for (n = 0; n < nreturns; n++) {
		char argname[80];
		snprintf(argname, sizeof(argname), "ret%zd", n + 1);
		const jive_resource_class * cls;
		switch (n) {
			case 0: cls = &jive_i386_regcls[jive_i386_gpr_eax].base; break;
			default: cls = jive_fixed_stackslot_class_get(4, 4, n * 4);
		}
		self->base.returns[n] = jive_resource_class_create_gate(cls, graph, argname);
	}
	
	jive_subroutine_create_region_and_nodes(&self->base, region);
	
	self->saved_esp = jive_subroutine_create_passthrough(&self->base, &jive_i386_regcls[jive_i386_gpr_esp].base, "saved_esp");
	self->saved_esp.gate->may_spill = false;
	self->saved_ebx = jive_subroutine_create_passthrough(&self->base, &jive_i386_regcls[jive_i386_gpr_ebx].base, "saved_ebx");
	self->saved_ebp = jive_subroutine_create_passthrough(&self->base, &jive_i386_regcls[jive_i386_gpr_ebp].base, "saved_ebp");
	self->saved_esi = jive_subroutine_create_passthrough(&self->base, &jive_i386_regcls[jive_i386_gpr_esi].base, "saved_esi");
	self->saved_edi = jive_subroutine_create_passthrough(&self->base, &jive_i386_regcls[jive_i386_gpr_edi].base, "saved_edi");
	
	self->stackptr = self->saved_esp.output;
	self->frameptr = self->saved_esp.output;
	
	/* return instruction */
	jive_node * ret_instr = jive_instruction_node_create(self->base.region, &jive_i386_instructions[jive_i386_ret], NULL, NULL);
	
	/* add dependency on return address on stack */
	const jive_resource_class * stackslot_cls = jive_fixed_stackslot_class_get(4, 4, 0);
	const jive_type * memory_state_type = jive_resource_class_get_type(stackslot_cls);
	jive_output * retaddr_def = jive_node_add_output(&self->base.enter->base, memory_state_type);
	retaddr_def->required_rescls = stackslot_cls;
	jive_input * retaddr_use = jive_node_add_input(ret_instr, memory_state_type, retaddr_def);
	retaddr_use->required_rescls = stackslot_cls;
	
	/* divert control output of "leave" node */
	JIVE_DECLARE_CONTROL_TYPE(control_type);
	jive_input_divert_origin(self->base.leave->base.inputs[0], jive_node_add_output(ret_instr, control_type));
	

	return &self->base;
}
