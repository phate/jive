/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/backend/i386/subroutine.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/addresstype.h>
#include <jive/arch/stackslot.h>
#include <jive/arch/subroutine/nodes.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/registerset.h>
#include <jive/types/function/fctlambda.h>
#include <jive/vsdg.h>
#include <jive/vsdg/splitnode.h>
#include <jive/vsdg/substitution.h>

/* convert according to "default" ABI */
jive::node *
jive_i386_subroutine_convert(jive::region * target_parent, jive::node * lambda_node)
{
	/* FIXME: this function is broken */
	return nullptr;
#if 0
	jive::region * src_region;
	src_region = dynamic_cast<jive::output*>(lambda_node->input(0)->origin())->node()->region();
	
	size_t nparameters = src_region->narguments();
	size_t nreturns = src_region->bottom()->ninputs()-1;
	
	size_t nvalue_parameters = 0, nstate_parameters = 0;
	size_t nvalue_returns = 0, nstate_returns = 0;
	jive_argument_type value_parameters[nparameters];
	jive_argument_type value_returns[nreturns];
	
	size_t n;
	for (n = 0; n < nparameters; n++) {
		jive::output * param = dynamic_cast<jive::output*>(src_region->top()->output(n+1));
		if (dynamic_cast<const jive::value::type*>(&param->type())) {
			value_parameters[nvalue_parameters ++] = jive_argument_long; /* FIXME: pick correct type */
		} else {
			nstate_parameters ++;
		}
	}
	for (n = 0; n < nreturns; n++) {
		jive::input * ret = dynamic_cast<jive::input*>(src_region->bottom()->input(n+1));
		if (dynamic_cast<const jive::value::type*>(&ret->type())) {
			value_returns[nvalue_returns ++] = jive_argument_long; /* FIXME: pick correct type */
		} else {
			nstate_returns ++;
		}
	}
	
	jive_subroutine sub = jive_i386_subroutine_begin(
		target_parent->graph(),
		nvalue_parameters, value_parameters,
		nvalue_returns, value_returns);

	jive::substitution_map subst;

	/* map all parameters */
	nvalue_parameters = 0;
	for (n = 1; n < src_region->top()->noutputs(); n++) {
		jive::output * original = dynamic_cast<jive::output*>(src_region->top()->output(n));
		
		jive::output * substitute;
		if (dynamic_cast<const jive::value::type*>(&original->type())) {
			substitute = dynamic_cast<jive::output*>(
				jive_subroutine_simple_get_argument(sub, nvalue_parameters ++));
		} else {
			substitute = dynamic_cast<jive::output*>(sub.region->top()->add_output(&original->type()));
		}
		
		if(dynamic_cast<const jive::addr::type*>(&original->type()))
			substitute = dynamic_cast<jive::output*>(
				jive_bitstring_to_address_create(substitute, 32, &original->type()));
		subst.insert(original, substitute);
	}
	
	/* transfer function body */
	src_region->copy(sub.region, subst, false, false);
	
	/* map all returns */
	nvalue_returns = 0;
	for (n = 1; n < src_region->bottom()->ninputs(); n++) {
		jive::input * original = dynamic_cast<jive::input*>(src_region->bottom()->input(n));
		jive::output * retval = dynamic_cast<jive::output*>(
			subst.lookup(src_region->bottom()->input(n)->origin()));
		
		if (dynamic_cast<const jive::value::type*>(&original->type())) {
			if(dynamic_cast<const jive::addr::type*>(&original->type()))
				retval = dynamic_cast<jive::output*>(
					jive_address_to_bitstring_create(retval, 32, &retval->type()));
			jive_subroutine_simple_set_result(sub, nvalue_returns ++, retval);
		} else {
			/* FIXME: state returns currently unsupported */
			JIVE_DEBUG_ASSERT(false);
		}
	}

	return jive_subroutine_end(sub);
#endif
}

static void
jive_i386_subroutine_prepare_stackframe_(
	const jive::subroutine_op & op,
	jive::region * region,
	jive_subroutine_stackframe_info * frame,
	const jive_subroutine_late_transforms * xfrm);

static jive::simple_input *
jive_i386_subroutine_add_fp_dependency_(
	const jive::subroutine_op & op, jive::region * region, jive::node * node);

static jive::simple_input *
jive_i386_subroutine_add_sp_dependency_(
	const jive::subroutine_op & op, jive::region * region, jive::node * node);

const jive_subroutine_abi_class JIVE_I386_SUBROUTINE_ABI = {
	prepare_stackframe : jive_i386_subroutine_prepare_stackframe_,
	add_fp_dependency : jive_i386_subroutine_add_fp_dependency_,
	add_sp_dependency : jive_i386_subroutine_add_sp_dependency_,
	instructionset : &jive_i386_instructionset
};

namespace {

class i386_c_builder_interface final : public jive::subroutine_hl_builder_interface {
public:
	virtual
	~i386_c_builder_interface() noexcept
	{
	}

	virtual jive::output *
	value_parameter(
		jive_subroutine & subroutine,
		size_t index) override
	{
		auto o = subroutine.builder_state->arguments[index].output;
		auto node = jive_splitnode_create(subroutine.region, o, o->port().gate()->rescls(),
			&jive_i386_regcls_gpr);
		return node->output(0);
	}

	virtual void
	value_return(
		jive_subroutine & subroutine,
		size_t index,
		jive::simple_output * value) override
	{
		subroutine.builder_state->results[index].output = value;
	}
	
	virtual jive::simple_output *
	finalize(
		jive_subroutine & subroutine) override
	{
		auto ret_instr = create_instruction(subroutine.region, &jive::i386::instr_ret::instance(),
			{}, {}, {jive::ctl::boolean});
		/* add dependency on return address on stack */
			ret_instr->add_input(subroutine.builder_state->passthroughs[6].gate,
				subroutine.builder_state->passthroughs[6].output);
		return dynamic_cast<jive::simple_output*>(ret_instr->output(0));
	}
};

}

jive_subroutine
jive_i386_subroutine_begin(jive::graph * graph,
	size_t nparameters, const jive_argument_type parameter_types[],
	size_t nreturns, const jive_argument_type return_types[])
{
	jive::subroutine_machine_signature sig;
	for (size_t n = 0; n < nparameters; n++) {
		auto argname = jive::detail::strfmt("arg", n+1);
		auto cls = jive_fixed_stackslot_class_get(4, 4, (n + 1) * 4);
		sig.arguments.emplace_back(jive::subroutine_machine_signature::argument{argname, cls, true});
	}
	
	for (size_t n = 0; n < nreturns; n++) {
		const jive::resource_class * cls;
		auto resname = jive::detail::strfmt("ret", n+1);
		switch (n) {
			case 0: cls = &jive_i386_regcls_gpr_eax; break;
			default: cls = jive_fixed_stackslot_class_get(4, 4, n * 4);
		}
		sig.results.emplace_back(jive::subroutine_machine_signature::result{resname, cls});
	}
	
	auto stackslot_cls = jive_fixed_stackslot_class_get(4, 4, 0);

	typedef jive::subroutine_machine_signature::passthrough pt;
	sig.passthroughs.emplace_back(pt{"mem", nullptr, false});
	sig.passthroughs.emplace_back(pt{"saved_esp", &jive_i386_regcls_gpr_esp, false});
	sig.passthroughs.emplace_back(pt{"saved_ebx", &jive_i386_regcls_gpr_ebx, true});
	sig.passthroughs.emplace_back(pt{"saved_ebp", &jive_i386_regcls_gpr_ebp, true});
	sig.passthroughs.emplace_back(pt{"saved_esi", &jive_i386_regcls_gpr_esi, true});
	sig.passthroughs.emplace_back(pt{"saved_edi", &jive_i386_regcls_gpr_edi, true});
	sig.passthroughs.emplace_back(pt{"return_addr", stackslot_cls, false});
	
	sig.stack_frame_upper_bound = 4 * (nparameters + 1);
	
	sig.abi_class = &JIVE_I386_SUBROUTINE_ABI;
	
	std::unique_ptr<jive::subroutine_hl_builder_interface> builder(
		new i386_c_builder_interface());
	return jive_subroutine_begin(
		graph,
		std::move(sig),
		std::move(builder));
}

typedef struct jive_i386_stackptr_split_factory {
	jive_value_split_factory base;
	ssize_t offset;
} jive_i386_stackptr_split_factory;

static jive::output *
do_stackptr_sub(const jive_value_split_factory * self_, jive::output * value)
{
	const jive_i386_stackptr_split_factory * self = (const jive_i386_stackptr_split_factory *) self_;
	jive::immediate imm(self->offset);

	auto tmp = jive_immediate_create(value->node()->region(), &imm);
	return jive::create_instruction(value->node()->region(),
		&jive::i386::instr_int_sub_immediate::instance(), {value, tmp})->output(0);
}

static jive::output *
do_stackptr_add(const jive_value_split_factory * self_, jive::output * value)
{
	const jive_i386_stackptr_split_factory * self = (const jive_i386_stackptr_split_factory *) self_;
	jive::immediate imm(self->offset);

	auto tmp = jive_immediate_create(value->node()->region(), &imm);
	return jive::create_instruction(value->node()->region(),
		&jive::i386::instr_int_add_immediate::instance(), {value, tmp})->output(0);
}

static void
jive_i386_subroutine_prepare_stackframe_(
	const jive::subroutine_op & op,
	jive::region * region,
	jive_subroutine_stackframe_info * frame,
	const jive_subroutine_late_transforms * xfrm)
{
	frame->lower_bound -= frame->call_area_size;
	
	if (!frame->lower_bound)
		return;
	
	frame->lower_bound = ((frame->lower_bound - 4) & ~15) + 4;
	
	jive_i386_stackptr_split_factory stackptr_sub = {
		{do_stackptr_sub},
		- frame->lower_bound
	};
	jive_i386_stackptr_split_factory stackptr_add = {
		{do_stackptr_add},
		- frame->lower_bound
	};
	
	/* as long as no frame pointer is used, access to stack slots through stack
	pointer must be relocated */
	frame->frame_pointer_offset += frame->lower_bound;
	
	xfrm->value_split(
		xfrm,
		op.get_passthrough_enter_by_index(region, 1),
		op.get_passthrough_leave_by_index(region, 1),
		&stackptr_sub.base,
		&stackptr_add.base);
}

static jive::simple_input *
jive_i386_subroutine_add_fp_dependency_(
	const jive::subroutine_op & op, jive::region * region, jive::node * node)
{
	auto frameptr = op.get_passthrough_enter_by_index(region, 1);
	
	size_t n;
	for (n = 0; n < node->ninputs(); n++) {
		auto input = dynamic_cast<jive::simple_input*>(node->input(n));
		if (input->origin() == frameptr)
			return NULL;
	}
	return dynamic_cast<jive::simple_input*>(node->add_input(frameptr->type(), frameptr));
}

static jive::simple_input *
jive_i386_subroutine_add_sp_dependency_(
	const jive::subroutine_op & op, jive::region * region, jive::node * node)
{
	auto stackptr = op.get_passthrough_enter_by_index(region, 1);
	
	size_t n;
	for (n = 0; n < node->ninputs(); n++) {
		auto input = dynamic_cast<jive::simple_input*>(node->input(n));
		if (input->origin() == stackptr)
			return NULL;
	}
	return dynamic_cast<jive::simple_input*>(node->add_input(stackptr->type(), stackptr));
}
