/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/instruction-private.h>

#include <jive/arch/compilate.h>
#include <jive/arch/immediate-type.h>
#include <jive/arch/immediate-node.h>
#include <jive/arch/label-mapper.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/traverser.h>
#include <jive/vsdg/sequence.h>

#include <stdio.h>
#include <string.h>

const jive_node_class JIVE_INSTRUCTION_NODE = {
	.parent = &JIVE_NODE,
	.name = "INSTRUCTION",
	.fini = jive_node_fini_, /* inherit */
	.get_default_normal_form = jive_node_get_default_normal_form_, /* inherit */
	.get_label = jive_instruction_node_get_label_, /* override */
	.get_attrs = jive_instruction_node_get_attrs_, /* override */
	.match_attrs = jive_instruction_node_match_attrs_, /* override */
	.create = jive_instruction_node_create_, /* override */
	.get_aux_rescls = jive_instruction_node_get_aux_rescls_, /* override */
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
	JIVE_DECLARE_CONTROL_TYPE(ctl);
	JIVE_DECLARE_IMMEDIATE_TYPE(imm);
	
	const jive_type * input_types[ninputs + nimmediates];
	jive_output * inputs[ninputs + nimmediates];
	const jive_type * output_types[noutputs];
	
	size_t n;
	for (n = 0; n < icls->ninputs; ++n) {
		input_types[n] = jive_register_class_get_type(icls->inregs[n]);
		inputs[n] = operands[n];
	}
	for (n = 0; n < icls->nimmediates; ++n) {
		input_types[n + ninputs] = imm;
		inputs[n + ninputs] = immediates[n];
	}
	for (n = 0; n < icls->noutputs; n++)
		output_types[n] = jive_register_class_get_type(icls->outregs[n]);
	
	if (icls->flags & jive_instruction_jump) {
		output_types[icls->noutputs] = ctl;
	}
	
	jive_node_init_(&self->base,
		region,
		ninputs + nimmediates, input_types, inputs,
		noutputs, output_types);
	
	for (n = 0; n<icls->ninputs; n++)
		self->base.inputs[n]->required_rescls = &icls->inregs[n]->base;
	for (n = 0; n<icls->noutputs; n++)
		self->base.outputs[n]->required_rescls = &icls->outregs[n]->base;
	
	self->attrs.icls = icls;
}

char *
jive_instruction_node_get_label_(const jive_node * self_)
{
	const jive_instruction_node * self = (const jive_instruction_node *) self_;
	return strdup(self->attrs.icls->name);
}

const jive_node_attrs *
jive_instruction_node_get_attrs_(const jive_node * self_)
{
	const jive_instruction_node * self = (const jive_instruction_node *) self_;
	return &self->attrs.base;
}

bool
jive_instruction_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_instruction_node_attrs * first = &((const jive_instruction_node *) self)->attrs;
	const jive_instruction_node_attrs * second = (const jive_instruction_node_attrs *) attrs;
	
	return first->icls == second->icls;
}

jive_node *
jive_instruction_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	const jive_instruction_node_attrs * attrs = (const jive_instruction_node_attrs *) attrs_;
	
	jive_instruction_node * other = jive_context_malloc(region->graph->context, sizeof(*other));
	other->base.class_ = &JIVE_INSTRUCTION_NODE;
	jive_instruction_node_init_(other, region, attrs->icls, operands, operands + attrs->icls->ninputs);
	
	return &other->base;
}

const struct jive_resource_class *
jive_instruction_node_get_aux_rescls_(const jive_node * self_)
{
	const jive_instruction_node * self = (const jive_instruction_node *) self_;
	
	if (self->attrs.icls->flags & jive_instruction_commutative) return 0;
	if (!(self->attrs.icls->flags & jive_instruction_write_input)) return 0;
	return &self->attrs.icls->inregs[0]->base;
}

jive_node *
jive_instruction_node_create_simple(
	jive_region * region,
	const jive_instruction_class * icls,
	jive_output * operands[const],
	const int64_t immediates[const])
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
	jive_output * operands[const],
	const jive_immediate immediates[])
{
	jive_output * immvalues[icls->nimmediates];
	size_t n;
	for (n = 0; n < icls->nimmediates; ++n) {
		immvalues[n] = jive_immediate_create(region->graph, &immediates[n]);
	}
	jive_instruction_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->base.class_ = &JIVE_INSTRUCTION_NODE;
	jive_instruction_node_init_(node, region, icls, operands, immvalues);
	
	return &node->base;
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
	.name = "PSEUDO_NOP",
	.mnemonic = "",
	.encode = jive_encode_PSEUDO_NOP,
	.write_asm = jive_write_asm_PSEUDO_NOP,
	.inregs = 0, .outregs = 0, .flags = jive_instruction_flags_none, .ninputs = 0, .noutputs = 0, .nimmediates = 0
};

static void
jive_seq_instruction_fini_(jive_seq_point * self_)
{
	jive_seq_instruction * self = (jive_seq_instruction *) self_;
	jive_context * context = self->base.seq_region->seq_graph->context;
	jive_context_free(context, self->instr.inputs);
	jive_context_free(context, self->instr.outputs);
	jive_context_free(context, self->instr.immediates);
	jive_seq_point_fini_(&self->base);
}

const jive_seq_point_class JIVE_SEQ_INSTRUCTION = {
	.parent = &JIVE_SEQ_POINT,
	.fini = jive_seq_instruction_fini_
};

static jive_seq_instruction *
jive_seq_instruction_create(
	jive_seq_region * seq_region,
	const jive_instruction_class * icls,
	const jive_register_name * const * inputs,
	const jive_register_name * const * outputs,
	const jive_immediate immediates[])
{
	jive_seq_graph * seq = seq_region->seq_graph;
	jive_context * context = seq->context;
	
	jive_seq_instruction * seq_instr = jive_context_malloc(context, sizeof(*seq_instr));
	seq_instr->base.class_ = &JIVE_SEQ_INSTRUCTION;
	jive_seq_point_init(&seq_instr->base, seq_region);
	seq_instr->instr.icls = icls;
	seq_instr->instr.inputs = jive_context_malloc(context, icls->ninputs * sizeof(seq_instr->instr.inputs[0]));
	seq_instr->instr.outputs = jive_context_malloc(context, icls->noutputs * sizeof(seq_instr->instr.outputs[0]));
	seq_instr->instr.immediates = jive_context_malloc(context, icls->nimmediates * sizeof(seq_instr->instr.immediates[0]));
	seq_instr->flags = 0;
	
	size_t n;
	for (n = 0; n < icls->ninputs; n++)
		seq_instr->instr.inputs[n] = inputs[n];
	for (n = 0; n < icls->noutputs; n++)
		seq_instr->instr.outputs[n] = outputs[n];
	for (n = 0; n < icls->nimmediates; n++)
		seq_instr->instr.immediates[n] = immediates[n];
	
	return seq_instr;
}

jive_seq_instruction *
jive_seq_instruction_create_before(
	jive_seq_point * before,
	const jive_instruction_class * icls,
	const jive_register_name * const * inputs,
	const jive_register_name * const * outputs,
	const jive_immediate immediates[])
{
	jive_seq_graph * seq = before->seq_region->seq_graph;
	jive_seq_instruction * seq_instr = jive_seq_instruction_create(
		before->seq_region, icls, inputs, outputs, immediates);
	
	JIVE_LIST_INSERT(seq->points, before, &seq_instr->base, seqpoint_list);
	if (before->seq_region->first_point == before)
		before->seq_region->first_point = &seq_instr->base;
	
	return seq_instr;
}

jive_seq_instruction *
jive_seq_instruction_create_after(
	jive_seq_point * after,
	const jive_instruction_class * icls,
	const jive_register_name * const * inputs,
	const jive_register_name * const * outputs,
	const jive_immediate immediates[])
{
	jive_seq_graph * seq = after->seq_region->seq_graph;
	jive_seq_instruction * seq_instr = jive_seq_instruction_create(
		after->seq_region, icls, inputs, outputs, immediates);
	
	JIVE_LIST_INSERT(seq->points, after->seqpoint_list.next, &seq_instr->base, seqpoint_list);
	
	return seq_instr;
}
