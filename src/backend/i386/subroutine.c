/*
 * Copyright 2010 2011 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/backend/i386/subroutine.h>

#include <stdio.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/addresstype.h>
#include <jive/arch/stackslot.h>
#include <jive/arch/subroutine-private.h>
#include <jive/arch/subroutine/nodes.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/registerset.h>
#include <jive/types/function/fctlambda.h>
#include <jive/vsdg.h>
#include <jive/vsdg/splitnode.h>
#include <jive/vsdg/substitution.h>

static jive_subroutine_deprecated *
jive_i386_subroutine_create(jive_region * region,
	size_t nparameters, const jive_argument_type parameter_types[],
	size_t nreturns, const jive_argument_type return_types[]);

/* convert according to "default" ABI */
jive_node *
jive_i386_subroutine_convert(jive_region * target_parent, jive_node * lambda_node)
{
	jive_region * src_region = lambda_node->producer(0)->region;
	jive_context * context = target_parent->graph->context;
	
	size_t nparameters = src_region->top->noutputs - 1;
	size_t nreturns = src_region->bottom->ninputs - 1;
	
	size_t nvalue_parameters = 0, nstate_parameters = 0;
	size_t nvalue_returns = 0, nstate_returns = 0;
	jive_argument_type value_parameters[nparameters];
	jive_argument_type value_returns[nreturns];
	
	size_t n;
	for (n = 0; n < nparameters; n++) {
		jive::output * param = src_region->top->outputs[n + 1];
		if (dynamic_cast<jive::value::output*>(param)) {
			value_parameters[nvalue_parameters ++] = jive_argument_long; /* FIXME: pick correct type */
		} else {
			nstate_parameters ++;
		}
	}
	for (n = 0; n < nreturns; n++) {
		jive::input * ret = src_region->bottom->inputs[n + 1];
		if (dynamic_cast<jive::value::input*>(ret)) {
			value_returns[nvalue_returns ++] = jive_argument_long; /* FIXME: pick correct type */
		} else {
			nstate_returns ++;
		}
	}
	
	jive_subroutine_deprecated * subroutine = jive_i386_subroutine_create(target_parent,
		nvalue_parameters, value_parameters,
		nvalue_returns, value_returns);

	jive_substitution_map * subst = jive_substitution_map_create(context);
	
	/* map all parameters */
	nvalue_parameters = 0;
	for (n = 1; n < src_region->top->noutputs; n++) {
		jive::output * original = src_region->top->outputs[n];
		
		jive::output * substitute;
		if (dynamic_cast<jive::value::output*>(original)) {
			substitute = jive_subroutine_value_parameter(subroutine, nvalue_parameters ++);
		} else {
			substitute = jive_node_add_output(subroutine->enter, &original->type());
		}
		
		if(dynamic_cast<jive::addr::output*>(original))
			substitute = jive_bitstring_to_address_create(substitute, 32, &original->type());
		jive_substitution_map_add_output(subst, original, substitute);
	}
	
	/* transfer function body */
	jive_region_copy_substitute(src_region, subroutine->region, subst, false, false);
	
	/* map all returns */
	nvalue_returns = 0;
	for (n = 1; n < src_region->bottom->ninputs; n++) {
		jive::input * original = src_region->bottom->inputs[n];
		jive::output * retval = jive_substitution_map_lookup_output(
			subst, src_region->bottom->inputs[n]->origin());
		
		if (dynamic_cast<jive::value::input*>(original)) {
			if(dynamic_cast<jive::addr::input*>(original))
				retval = jive_address_to_bitstring_create(retval, 32, &retval->type());
			jive_subroutine_value_return(subroutine, nvalue_returns ++, retval);
		} else {
			jive_node_add_input(subroutine->leave, &original->type(), retval);
		}
	}
	
	jive_substitution_map_destroy(subst);
	
	return subroutine->subroutine_node;
}

static jive::output *
jive_i386_subroutine_value_parameter_(jive_subroutine_deprecated * self_, size_t index);

static jive::input *
jive_i386_subroutine_value_return_(jive_subroutine_deprecated * self_, size_t index,
	jive::output * value);

static void
jive_i386_subroutine_prepare_stackframe_(
	jive_subroutine_deprecated * self_, const jive_subroutine_late_transforms * xfrm);

static jive::input *
jive_i386_subroutine_add_fp_dependency_(const jive_subroutine_deprecated * self, jive_node * node);

static jive::input *
jive_i386_subroutine_add_sp_dependency_(const jive_subroutine_deprecated * self, jive_node * node);

const jive_subroutine_class JIVE_I386_SUBROUTINE = {
	fini : jive_subroutine_fini_,
	value_parameter : jive_i386_subroutine_value_parameter_,
	value_return : jive_i386_subroutine_value_return_,
};

const jive_subroutine_abi_class JIVE_I386_SUBROUTINE_ABI = {
	prepare_stackframe : jive_i386_subroutine_prepare_stackframe_,
	add_fp_dependency : jive_i386_subroutine_add_fp_dependency_,
	add_sp_dependency : jive_i386_subroutine_add_sp_dependency_,
	instructionset : &jive_i386_instructionset
};

static jive::output *
jive_i386_subroutine_value_parameter_(jive_subroutine_deprecated * self_, size_t index)
{
	jive_i386_subroutine * self = (jive_i386_subroutine *) self_;
	jive::gate * gate = self->base.parameters[index];
	jive::output * output = jive_node_gate_output(self->base.enter, gate);
	
	const jive::base::type * in_type = &gate->type();
	const jive::base::type * out_type = jive_resource_class_get_type(&jive_i386_regcls_gpr.base);
	jive_node * node = jive_splitnode_create(self->base.enter->region,
		in_type, output, gate->required_rescls,
		out_type, &jive_i386_regcls_gpr.base);
	output = node->outputs[0];
	
	return output;
}

static jive::input *
jive_i386_subroutine_value_return_(jive_subroutine_deprecated * self_, size_t index,
	jive::output * value)
{
	jive_i386_subroutine * self = (jive_i386_subroutine *) self_;
	jive::gate * gate = self->base.returns[index];
	return jive_node_gate_input(self->base.leave, gate, value);
}

static jive_subroutine_deprecated *
jive_i386_subroutine_create(jive_region * region,
	size_t nparameters, const jive_argument_type parameter_types[],
	size_t nreturns, const jive_argument_type return_types[])
{
	jive_graph * graph = region->graph;
	jive_context * context = graph->context;
	jive_i386_subroutine * self = jive_context_malloc(context, sizeof(*self));
	jive_subroutine_init_(&self->base, &JIVE_I386_SUBROUTINE, context,
		nparameters, parameter_types, nreturns, return_types, 6);
	self->base.abi_class = &JIVE_I386_SUBROUTINE_ABI;
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
			case 0: cls = &jive_i386_regcls_gpr_eax.base; break;
			default: cls = jive_fixed_stackslot_class_get(4, 4, n * 4);
		}
		self->base.returns[n] = jive_resource_class_create_gate(cls, graph, argname);
	}
	
	jive_subroutine_create_region_and_nodes(&self->base, region);
	
	self->base.passthroughs[0] = jive_subroutine_create_passthrough_memorystate(
		&self->base, "mem");
	self->base.passthroughs[1] = jive_subroutine_create_passthrough(
		&self->base, &jive_i386_regcls_gpr_esp.base, "saved_esp");
	self->base.passthroughs[1].gate->may_spill = false;
	self->base.passthroughs[2] = jive_subroutine_create_passthrough(
		&self->base, &jive_i386_regcls_gpr_ebx.base, "saved_ebx");
	self->base.passthroughs[3] = jive_subroutine_create_passthrough(
		&self->base, &jive_i386_regcls_gpr_ebp.base, "saved_ebp");
	self->base.passthroughs[4] = jive_subroutine_create_passthrough(
		&self->base, &jive_i386_regcls_gpr_esi.base, "saved_esi");
	self->base.passthroughs[5] = jive_subroutine_create_passthrough(
		&self->base, &jive_i386_regcls_gpr_edi.base, "saved_edi");
	
	/* return instruction */
	jive_node * ret_instr = jive_instruction_node_create(
		self->base.region, &jive_i386_instr_ret, NULL, NULL);
	
	/* add dependency on return address on stack */
	const jive_resource_class * stackslot_cls = jive_fixed_stackslot_class_get(4, 4, 0);
	const jive::base::type * memory_state_type = jive_resource_class_get_type(stackslot_cls);
	jive::output * retaddr_def = jive_node_add_output(self->base.enter, memory_state_type);
	retaddr_def->required_rescls = stackslot_cls;
	jive::input * retaddr_use = jive_node_add_input(ret_instr, memory_state_type, retaddr_def);
	retaddr_use->required_rescls = stackslot_cls;
	
	/* divert control output of "leave" node */
	self->base.leave->inputs[0]->divert_origin(ret_instr->outputs[0]);

	return &self->base;
}

jive_subroutine
jive_i386_subroutine_begin(jive_graph * graph,
	size_t nparameters, const jive_argument_type parameter_types[],
	size_t nreturns, const jive_argument_type return_types[])
{
	jive_subroutine_deprecated * s = jive_i386_subroutine_create(
		graph->root_region,
		nparameters, parameter_types,
		nreturns, return_types);
	
	jive_subroutine sub = {
		region : s->region,
		old_subroutine_struct : s
	};
	
	return sub;
}

typedef struct jive_i386_stackptr_split_factory {
	jive_value_split_factory base;
	int offset;
} jive_i386_stackptr_split_factory;

static jive::output *
do_stackptr_sub(const jive_value_split_factory * self_, jive::output * value)
{
	const jive_i386_stackptr_split_factory * self = (const jive_i386_stackptr_split_factory *) self_;
	int64_t immediates[1] = {self->offset};
	
	return jive_instruction_node_create_simple(
		value->node()->region,
		&jive_i386_instr_int_sub_immediate,
		&value, immediates)->outputs[0];
}

static jive::output *
do_stackptr_add(const jive_value_split_factory * self_, jive::output * value)
{
	const jive_i386_stackptr_split_factory * self = (const jive_i386_stackptr_split_factory *) self_;
	int64_t immediates[1] = {self->offset};
	
	return jive_instruction_node_create_simple(
		value->node()->region,
		&jive_i386_instr_int_add_immediate,
		&value, immediates)->outputs[0];
}

static void
jive_i386_subroutine_prepare_stackframe_(
	jive_subroutine_deprecated * self_, const jive_subroutine_late_transforms * xfrm)
{
	jive_i386_subroutine * self = (jive_i386_subroutine *) self_;
	self->base.frame.lower_bound -= self->base.frame.call_area_size;
	
	if (!self->base.frame.lower_bound)
		return;
	
	self->base.frame.lower_bound = ((self->base.frame.lower_bound - 4) & ~15) + 4;
	
	jive_i386_stackptr_split_factory stackptr_sub = {
		{do_stackptr_sub},
		- self->base.frame.lower_bound
	};
	jive_i386_stackptr_split_factory stackptr_add = {
		{do_stackptr_add},
		- self->base.frame.lower_bound
	};
	
	/* as long as no frame pointer is used, access to stack slots through stack
	pointer must be relocated */
	self->base.frame.frame_pointer_offset += self->base.frame.lower_bound;
	
	xfrm->value_split(xfrm, self->base.passthroughs[1].output,
		self->base.passthroughs[1].input, &stackptr_sub.base, &stackptr_add.base);
}

static jive::input *
jive_i386_subroutine_add_fp_dependency_(const jive_subroutine_deprecated * self_, jive_node * node)
{
	jive_i386_subroutine * self = (jive_i386_subroutine *) self_;
	jive::output * frameptr = self->base.passthroughs[1].output;
	
	size_t n;
	for (n = 0; n < node->ninputs; n++) {
		jive::input * input = node->inputs[n];
		if (input->origin() == frameptr)
			return NULL;
	}
	return jive_node_add_input(node, &frameptr->type(), frameptr);
}

static jive::input *
jive_i386_subroutine_add_sp_dependency_(const jive_subroutine_deprecated * self_, jive_node * node)
{
	jive_i386_subroutine * self = (jive_i386_subroutine *) self_;
	jive::output * stackptr = self->base.passthroughs[1].output;
	
	size_t n;
	for (n = 0; n < node->ninputs; n++) {
		jive::input * input = node->inputs[n];
		if (input->origin() == stackptr)
			return NULL;
	}
	return jive_node_add_input(node, &stackptr->type(), stackptr);
}
