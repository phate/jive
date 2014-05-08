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

const jive_node_class JIVE_INSTRUCTION_NODE = {
	parent : &JIVE_NODE,
	name : "INSTRUCTION",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : jive_instruction_node_get_label_, /* override */
	get_attrs : nullptr,
	match_attrs : jive_instruction_node_match_attrs_, /* override */
	check_operands : jive_node_check_operands_, /* inherit */
	create : jive_instruction_node_create_, /* override */
};

void
jive_instruction_node_init_(
	jive_instruction_node * self,
	jive_region * region,
	const jive_instruction_class * icls,
	jive_output * const operands[],
	jive_output * const immediates[])
{
	size_t ninputs = icls->ninputs;
	size_t nimmediates = icls->nimmediates;
	size_t noutputs = icls->noutputs;
	if (icls->flags & jive_instruction_jump)
		noutputs ++;
	jive_control_type ctl;
	jive_immediate_type imm;
	
	const jive_type * input_types[ninputs + nimmediates];
	jive_output * inputs[ninputs + nimmediates];
	const jive_type * output_types[noutputs];
	
	size_t n;
	for (n = 0; n < icls->ninputs; ++n) {
		input_types[n] = jive_register_class_get_type(icls->inregs[n]);
		inputs[n] = operands[n];
	}
	for (n = 0; n < icls->nimmediates; ++n) {
		input_types[n + ninputs] = &imm;
		inputs[n + ninputs] = immediates[n];
	}
	for (n = 0; n < icls->noutputs; n++)
		output_types[n] = jive_register_class_get_type(icls->outregs[n]);
	
	if (icls->flags & jive_instruction_jump) {
		output_types[icls->noutputs] = &ctl;
	}
	
	jive_node_init_(self,
		region,
		ninputs + nimmediates, input_types, inputs,
		noutputs, output_types);
	
	for (n = 0; n<icls->ninputs; n++)
		self->inputs[n]->required_rescls = &icls->inregs[n]->base;
	for (n = 0; n<icls->noutputs; n++)
		self->outputs[n]->required_rescls = &icls->outregs[n]->base;
}

void
jive_instruction_node_get_label_(const jive_node * self_, struct jive_buffer * buffer)
{
	const jive_instruction_node * self = (const jive_instruction_node *) self_;
	jive_buffer_putstr(buffer, self->operation().icls()->name);
}

bool
jive_instruction_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive::instruction_operation * first = &((const jive_instruction_node *) self)->operation();
	const jive::instruction_operation * second = (const jive::instruction_operation *) attrs;
	
	return first->icls() == second->icls();
}

jive_node *
jive_instruction_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	const jive::instruction_operation * attrs = (const jive::instruction_operation *) attrs_;
	
	jive_instruction_node * other = new jive_instruction_node(*attrs);
	other->class_ = &JIVE_INSTRUCTION_NODE;
	jive_instruction_node_init_(
		other, region, attrs->icls(),
		operands, operands + attrs->icls()->ninputs);
	
	return other;
}

jive_node *
jive_instruction_node_create_simple(
	jive_region * region,
	const jive_instruction_class * icls,
	jive_output * const * operands,
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
	jive_output * const * operands,
	const jive_immediate immediates[])
{
	jive_output * immvalues[icls->nimmediates];
	size_t n;
	for (n = 0; n < icls->nimmediates; ++n) {
		immvalues[n] = jive_immediate_create(region->graph, &immediates[n]);
	}
	jive_instruction_node * node = new jive_instruction_node(
		jive::instruction_operation(icls));
	node->class_ = &JIVE_INSTRUCTION_NODE;
	jive_instruction_node_init_(node, region, icls, operands, immvalues);
	
	return node;
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
