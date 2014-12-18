/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/instruction-private.h>

#include <jive/arch/compilate.h>
#include <jive/arch/immediate-node.h>
#include <jive/arch/immediate-type.h>
#include <jive/arch/label-mapper.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/traverser.h>

#include <stdio.h>
#include <string.h>

namespace jive {

instruction_op::~instruction_op() noexcept
{
}

bool
instruction_op::operator==(const operation & other) const noexcept
{
	const instruction_op * op =
		dynamic_cast<const instruction_op *>(&other);
	return op && op->icls() == icls();
}

size_t
instruction_op::narguments() const noexcept
{
	return icls()->ninputs + icls()->nimmediates;
}

const jive::base::type &
instruction_op::argument_type(size_t index) const noexcept
{
	static const jive::imm::type immediate_type;
	if (index < icls()->ninputs) {
		return *jive_register_class_get_type(icls()->inregs[index]);
	} else {
		return immediate_type;
	}
}

const jive_resource_class *
instruction_op::argument_cls(size_t index) const noexcept
{
	if (index < icls()->ninputs) {
		return &icls()->inregs[index]->base;
	} else {
		return &jive_root_resource_class;
	}
}

size_t
instruction_op::nresults() const noexcept
{
	return icls()->noutputs +
		((icls()->flags & jive_instruction_jump) ? 1 : 0);
}

const jive_resource_class *
instruction_op::result_cls(size_t index) const noexcept
{
	if (index < icls()->noutputs) {
		return &icls()->outregs[index]->base;
	} else {
		return &jive_root_resource_class;
	}
}

const jive::base::type &
instruction_op::result_type(size_t index) const noexcept
{
	static const jive::ctl::type control_type;
	if (index < icls()->noutputs) {
		return *jive_register_class_get_type(icls()->outregs[index]);
	} else {
		return control_type;
	}
}
std::string
instruction_op::debug_string() const
{
	return icls()->name;
}

std::unique_ptr<jive::operation>
instruction_op::copy() const
{
	return std::unique_ptr<jive::operation>(new instruction_op(*this));
}

}

jive_node *
jive_instruction_node_create_simple(
	jive_region * region,
	const jive_instruction_class * icls,
	jive::output * const * operands,
	const int64_t * immediates)
{
	jive_immediate imm[icls->nimmediates];
	size_t n;
	for (n = 0; n < icls->nimmediates; ++n)
		jive_immediate_init(&imm[n], immediates[0], NULL, NULL, NULL);
	return jive_instruction_node_create_extended(region, icls, operands, imm);
}

jive_node *
jive_instruction_node_create_extended(
	jive_region * region,
	const jive_instruction_class * icls,
	jive::output * const * operands,
	const jive_immediate immediates[])
{
	jive::output * arguments[icls->ninputs + icls->nimmediates];
	for (size_t n = 0; n < icls->ninputs; ++n) {
		arguments[n] = operands[n];
	}
	for (size_t n = 0; n < icls->nimmediates; ++n) {
		arguments[n + icls->ninputs] = jive_immediate_create(
			region->graph, &immediates[n]);
	}
	jive::instruction_op op(icls);
	return op.create_node(region, icls->ninputs + icls->nimmediates, arguments);
}

static void
jive_encode_PSEUDO_NOP(const jive_instruction_class * icls,
	jive_section * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_codegen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
}

static void
jive_write_asm_PSEUDO_NOP(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_asmgen_imm immediates[],
	jive_instruction_encoding_flags * flags)
{
}

const jive_instruction_class JIVE_PSEUDO_NOP = {
	name : "PSEUDO_NOP",
	mnemonic : "",
	encode : jive_encode_PSEUDO_NOP,
	write_asm : jive_write_asm_PSEUDO_NOP,
	inregs : 0,
	outregs : 0,
	flags : jive_instruction_flags_none,
	ninputs : 0,
	noutputs : 0,
	nimmediates : 0
};
