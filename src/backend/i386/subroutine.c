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
#include <jive/rvsdg.h>
#include <jive/rvsdg/splitnode.h>
#include <jive/rvsdg/substitution.h>

typedef struct jive_i386_stackptr_split_factory {
	jive_value_split_factory base;
	ssize_t offset;
} jive_i386_stackptr_split_factory;

static jive::output *
do_stackptr_sub(const jive_value_split_factory * self_, jive::output * value)
{
	const jive_i386_stackptr_split_factory * self = (const jive_i386_stackptr_split_factory *) self_;
	jive::immediate imm(self->offset);

	auto tmp = jive::immediate_op::create(value->node()->region(), imm);
	return jive::create_instruction(value->node()->region(),
		&jive::i386::instr_int_sub_immediate::instance(), {value, tmp})->output(0);
}

static jive::output *
do_stackptr_add(const jive_value_split_factory * self_, jive::output * value)
{
	const jive_i386_stackptr_split_factory * self = (const jive_i386_stackptr_split_factory *) self_;
	jive::immediate imm(self->offset);

	auto tmp = jive::immediate_op::create(value->node()->region(), imm);
	return jive::create_instruction(value->node()->region(),
		&jive::i386::instr_int_add_immediate::instance(), {value, tmp})->output(0);
}

class jive_i386_subroutine_abi final : public jive::subroutine_abi {
public:
	virtual
	~jive_i386_subroutine_abi()
	{}

private:
	inline constexpr
	jive_i386_subroutine_abi()
	: jive::subroutine_abi()
	{}

	jive_i386_subroutine_abi(const jive_i386_subroutine_abi &) = delete;

	jive_i386_subroutine_abi(jive_i386_subroutine_abi &&) = delete;

public:
	virtual void
	prepare_stackframe(
		const jive::subroutine_op & op,
		jive::region * region,
		jive_subroutine_stackframe_info * frame,
		const jive_subroutine_late_transforms * xform) const override
	{
		frame->lower_bound -= frame->call_area_size;
		if (!frame->lower_bound)
			return;

		frame->lower_bound = ((frame->lower_bound - 4) & ~15) + 4;
		jive_i386_stackptr_split_factory stackptr_sub = {{do_stackptr_sub}, - frame->lower_bound};
		jive_i386_stackptr_split_factory stackptr_add = {{do_stackptr_add}, - frame->lower_bound};

		/* as long as no frame pointer is used, access to stack slots through stack
		pointer must be relocated */
		frame->frame_pointer_offset += frame->lower_bound;

		xform->value_split(xform,
			op.get_passthrough_enter_by_index(region, 1),
			op.get_passthrough_leave_by_index(region, 1),
			&stackptr_sub.base,
			&stackptr_add.base);
	}

	virtual jive::input *
	add_fp_dependency(
		const jive::subroutine_op & op,
		jive::region * region,
		jive::node * node) const override
	{
		auto frameptr = op.get_passthrough_enter_by_index(region, 1);

		for (size_t n = 0; n < node->ninputs(); n++) {
			if (node->input(n)->origin() == frameptr)
				return nullptr;
		}

		/* FIXME */
		JIVE_ASSERT(0);
		return nullptr;
	}

	virtual jive::input *
	add_sp_dependency(
		const jive::subroutine_op & op,
		jive::region * region,
		jive::node * node) const override
	{
		auto stackptr = op.get_passthrough_enter_by_index(region, 1);

		for (size_t n = 0; n < node->ninputs(); n++) {
			if (node->input(n)->origin() == stackptr)
				return nullptr;
		}

		/* FIXME */
		JIVE_ASSERT(0);
		return nullptr;
	}

	virtual const jive::instructionset *
	instructionset() const noexcept override
	{
		return jive::i386::instructionset::get();
	}

	static inline const jive_i386_subroutine_abi *
	get()
	{
		static const jive_i386_subroutine_abi abi;
		return &abi;
	}
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
		return jive::split_op::create(o, o->port().gate()->rescls(), &jive::i386::gpr_regcls);
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
		auto ptgate = subroutine.builder_state->passthroughs[6].gate;
		auto ptoperand = subroutine.builder_state->passthroughs[6].output;
		auto ret_instr = create_instruction(subroutine.region, &jive::i386::instr_ret::instance(),
			{ptoperand}, {ptgate}, {jive::ctl2});
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
			case 0: cls = &jive::i386::eax_regcls; break;
			default: cls = jive_fixed_stackslot_class_get(4, 4, n * 4);
		}
		sig.results.emplace_back(jive::subroutine_machine_signature::result{resname, cls});
	}
	
	auto stackslot_cls = jive_fixed_stackslot_class_get(4, 4, 0);

	typedef jive::subroutine_machine_signature::passthrough pt;
	sig.passthroughs.emplace_back(pt{"mem", nullptr, false});
	sig.passthroughs.emplace_back(pt{"saved_esp", &jive::i386::esp_regcls, false});
	sig.passthroughs.emplace_back(pt{"saved_ebx", &jive::i386::ebx_regcls, true});
	sig.passthroughs.emplace_back(pt{"saved_ebp", &jive::i386::ebp_regcls, true});
	sig.passthroughs.emplace_back(pt{"saved_esi", &jive::i386::esi_regcls, true});
	sig.passthroughs.emplace_back(pt{"saved_edi", &jive::i386::edi_regcls, true});
	sig.passthroughs.emplace_back(pt{"return_addr", stackslot_cls, false});
	
	sig.stack_frame_upper_bound = 4 * (nparameters + 1);
	
	sig.abi_class = jive_i386_subroutine_abi::get();

	std::unique_ptr<jive::subroutine_hl_builder_interface> builder(
		new i386_c_builder_interface());
	return jive_subroutine_begin(
		graph,
		std::move(sig),
		std::move(builder));
}
