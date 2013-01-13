/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/codegen.h>

#include <stdio.h>
#include <string.h>

#include <jive/arch/dataobject.h>
#include <jive/arch/instruction.h>
#include <jive/arch/instructionset.h>
#include <jive/arch/subroutine.h>
#include <jive/types/bitstring/symbolic-expression.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/theta.h>
#include <jive/vsdg/variable.h>

static void
generate_code_for_instruction(jive_seq_point * seq_point,
	const jive_instruction * instr, jive_section * section, uint32_t * flags)
{
	const jive_instruction_class * icls = instr->icls;
	
	if ( (*flags & jive_instruction_encoding_flags_jump_conditional_invert) )
		icls = icls->inverse_jump;
	
	jive_immediate immediates[icls->nimmediates];
	size_t n;
	for (n = 0; n < icls->nimmediates; ++n)
		jive_immediate_assign(&instr->immediates[n], &immediates[n]);
	
	if (icls->flags & jive_instruction_jump_relative) {
		jive_immediate current;
		jive_immediate_init(&current, 0, &jive_label_current, 0, 0);
		immediates[0] = jive_immediate_sub(&immediates[0], &current);
	}
	
	for (n = 0; n < icls->nimmediates; ++n)
		jive_immediate_simplify(&immediates[n], seq_point);
	
	jive_instruction_encoding_flags iflags;
	iflags = (jive_instruction_encoding_flags) *flags;
	icls->encode(icls, section, instr->inputs, instr->outputs, immediates, &iflags);
	*flags = (uint32_t) iflags;
}

static void
jive_dataitem_generate_code(jive_node * data_item, jive_section * section)
{
	jive_bitconstant_node * node = jive_bitconstant_node_cast(data_item);
	if (node) {
		char * bits = node->attrs.bits;
		size_t nbits = node->attrs.nbits;
		JIVE_DEBUG_ASSERT((nbits % 8) == 0);

		//FIXME: endianess
		size_t n;
		uint8_t data = 0;
		for (n = 0; n < nbits; n++) {
			data |= (bits[n] - '0') << (n % 8);
			if ((n % 8) == 7) {
				jive_buffer_putbyte(&section->contents, data);
				data = 0;
			}
		}
	} else {
		jive_context_fatal_error(section->contents.context,
			"Type mismatch: don't know how to generate code for node");
	}
}

static void
jive_dataitems_node_generate_code(jive_seq_node * seq_node, jive_section * section)
{
	size_t n;
	for (n = 0; n < seq_node->node->ninputs; n++)
		jive_dataitem_generate_code(seq_node->node->inputs[n]->origin->node, section);
}

static void
jive_seq_node_generate_code(jive_seq_node * seq_node, jive_section * section)
{
	if (jive_node_isinstance(seq_node->node, &JIVE_INSTRUCTION_NODE)) {
		jive_instruction_node * instr_node = (jive_instruction_node *) seq_node->node;
		const jive_instruction_class * icls = instr_node->attrs.icls;
		
		size_t n;
		const jive_register_name * inregs[icls->ninputs];
		for (n = 0; n < icls->ninputs; n++) {
			const jive_resource_name * resname = jive_variable_get_resource_name(
				seq_node->node->inputs[n]->ssavar->variable);
			inregs[n] = (const jive_register_name *) resname;
		}
		
		const jive_register_name * outregs[icls->noutputs];
		for (n = 0; n < icls->noutputs; n++) {
			const jive_resource_name * resname = jive_variable_get_resource_name(
				seq_node->node->outputs[n]->ssavar->variable);
			outregs[n] = (const jive_register_name *) resname;
		}

		jive_instruction instr;
		instr.icls = icls;
		instr.inputs = inregs;
		instr.outputs = outregs;
		instr.immediates = instr_node->attrs.immediates;
		generate_code_for_instruction(&seq_node->base, &instr, section, &seq_node->flags);
	} else if (jive_node_isinstance(seq_node->node, &JIVE_DATAITEMS_NODE)) {
		jive_dataitems_node_generate_code(seq_node, section);
	}
}

static void
generate_code(jive_seq_graph * seq_graph, struct jive_compilate * compilate)
{
	jive_seq_point * seq_point;
	
	JIVE_LIST_ITERATE(seq_graph->points, seq_point, seqpoint_list) {
		jive_stdsectionid sectionid =
			jive_seq_point_map_to_section(seq_point->seq_region->first_point);
		jive_section * section = jive_compilate_get_standard_section(
			compilate, sectionid);
		if (!section) continue;

		jive_offset offset = section->contents.size;
		if (jive_seq_point_isinstance(seq_point, &JIVE_SEQ_NODE)) {
			jive_seq_node * seq_node = (jive_seq_node *) seq_point;
			jive_seq_node_generate_code(seq_node, section);
		} else if (jive_seq_point_isinstance(seq_point, &JIVE_SEQ_INSTRUCTION)) {
			jive_seq_instruction * seq_instr = (jive_seq_instruction *) seq_point;
			generate_code_for_instruction(&seq_instr->base, &seq_instr->instr, section,
				&seq_instr->flags);	
		} 

		size_t size = section->contents.size - offset;
		if (offset != seq_point->address.offset || size != seq_point->size) {
			jive_address_init(&seq_point->address, sectionid, offset);
			seq_point->size = size;
			seq_point->seq_region->seq_graph->addrs_changed = true;
		}
	}
}

void
jive_seq_graph_generate_code(jive_seq_graph * seq_graph, jive_compilate * buffer)
{
	/* redo until no labels change anymore; this is actually a bit too
	pessimistic, as we only need to redo if
	- a *forward* reference label may have changed AND
	- the encoding of at least one instruction depends on
	  the value of one of the changed labels */
	while (seq_graph->addrs_changed) {
		jive_compilate_clear(buffer);
		seq_graph->addrs_changed = false;
		generate_code(seq_graph, buffer);
	}
}

static bool
jive_label_points_to_node(
	const jive_seq_graph * seq_graph,
	const jive_label * label,
	const jive_seq_point * to)
{
	if (!jive_label_isinstance(label, &JIVE_LABEL_INTERNAL))
		return false;
	
	const jive_label_internal * label_int = (const jive_label_internal *) label;
	jive_seq_point * sp = jive_label_internal_get_attach_node(label_int, seq_graph);
	
	return sp == to;
}

/* for a conditional branch with two possibly different continuation points,
patch in unconditional branch when condition for primary target is not met */
static void
jive_seq_graph_patch_jump_targets(jive_seq_graph * seq_graph, jive_seq_node * seq_node, jive_instruction_node * inode)
{
	size_t index = inode->attrs.icls->noutputs;
	JIVE_DEBUG_ASSERT(inode->base.noutputs);
	jive_output * ctl_out = inode->base.outputs[index];
	JIVE_DEBUG_ASSERT(ctl_out->class_ == &JIVE_CONTROL_OUTPUT);
	
	JIVE_DEBUG_ASSERT(ctl_out->users.first == ctl_out->users.last);
	if (!ctl_out->users.first)
		return;
	jive_node * user = ctl_out->users.first->node;
	JIVE_DEBUG_ASSERT(user);
	
	jive_label * primary_tgt = 0, * secondary_tgt = 0;
	if (user->class_ == &JIVE_SUBROUTINE_LEAVE_NODE) {
		return;
	} else if (user->class_ == &JIVE_GAMMA_NODE) {
		jive_region * primary_region = user->inputs[0]->origin->node->region;
		jive_region * secondary_region = user->inputs[1]->origin->node->region;
		primary_tgt = jive_label_region_start_create(primary_region);
		secondary_tgt = jive_label_region_start_create(secondary_region);
	} else {
		JIVE_DEBUG_ASSERT(user->class_ == &JIVE_THETA_TAIL_NODE);
		jive_region * region = user->region;
		primary_tgt = jive_label_region_start_create(region);
		secondary_tgt = jive_label_region_end_create(region);
	}
	
	if ( (inode->attrs.icls->flags & jive_instruction_jump_conditional_invertible) ) {
		if (jive_label_points_to_node(seq_graph, primary_tgt, seq_node->base.seqpoint_list.next)) {
			primary_tgt = secondary_tgt;
			secondary_tgt = 0;
			seq_node->flags |= jive_instruction_encoding_flags_jump_conditional_invert;
		}
	}
	
	inode->attrs.immediates[0].add_label = primary_tgt;
	
	if (secondary_tgt) {
		const jive_instructionset * isa = jive_region_get_instructionset(inode->base.region);
		const jive_instruction_class * jump_icls = jive_instructionset_get_jump_instruction_class(isa);
		
		jive_immediate imm;
		jive_immediate_init(&imm, 0, secondary_tgt, 0, 0);
		jive_seq_instruction_create_after(&seq_node->base, jump_icls,
			NULL, NULL, &imm);
	}
}

/* if a region was taken "out-of-line", patch in a jump to return to the
anchor point */
static void
jive_seq_graph_patch_region_end(jive_seq_graph * seq_graph, jive_seq_node * seq_node)
{
	jive_seq_point * next = seq_node->base.seqpoint_list.next;
	jive_node * anchor_node = seq_node->node->outputs[0]->users.first->node;
	
	/* if next sequenced node is anchor node, then nothing to do */
	jive_seq_node * next_node = 0;
	if (next)
		next_node = jive_seq_node_cast(next);
	if (next_node && next_node->node == anchor_node)
		return;
	
	const jive_instructionset * isa = jive_region_get_instructionset(anchor_node->region);
	const jive_instruction_class * jump_icls = jive_instructionset_get_jump_instruction_class(isa);
	
	jive_label * tgt = jive_label_node_create(anchor_node);
	jive_immediate imm;
	jive_immediate_init(&imm, 0, tgt, 0, 0);
	jive_seq_instruction_create_after(&seq_node->base, jump_icls,
		NULL, NULL, &imm);
}

void
jive_seq_graph_patch_jumps(jive_seq_graph * seq_graph)
{
	jive_seq_point * seq_point;
	JIVE_LIST_ITERATE(seq_graph->points, seq_point, seqpoint_list) {
		jive_seq_node * seq_node = jive_seq_node_cast(seq_point);
		if (!seq_node)
			continue;
		jive_instruction_node * inode = jive_instruction_node_cast(seq_node->node);
		if (inode && (inode->attrs.icls->flags & jive_instruction_jump))
			jive_seq_graph_patch_jump_targets(seq_graph, seq_node, inode);
		
		if (seq_node->node == seq_node->node->region->bottom)
			jive_seq_graph_patch_region_end(seq_graph, seq_node);
	}
}

void
jive_graph_generate_code(jive_graph * graph, struct jive_compilate * buffer)
{
	jive_seq_graph * seq_graph = jive_graph_sequentialize(graph);
	jive_seq_graph_patch_jumps(seq_graph);
	
	jive_seq_graph_generate_code(seq_graph, buffer);
	
	jive_seq_graph_destroy(seq_graph);
}

static void
emit_region_start_attrs(const jive_region * region, jive_buffer * buffer)
{
	/* FIXME: should be based on *target* instead of *host*
	properties... hack for the sake of OS X, oh well... */
	#ifdef __ELF__
	switch (region->attrs.section) {
		default:
		case jive_region_section_inherit:
			break;
		case jive_region_section_code:
			jive_buffer_putstr(buffer, ".section .text\n");
			break;
		case jive_region_section_data:
			jive_buffer_putstr(buffer, ".section .data\n");
			break;
		case jive_region_section_rodata:
			jive_buffer_putstr(buffer, ".section .rodata\n");
			break;
		case jive_region_section_bss:
			jive_buffer_putstr(buffer, ".section .bss\n");
			break;
	};
	#else
	switch (region->attrs.section) {
		default:
		case jive_region_section_inherit:
			break;
		case jive_region_section_code:
			jive_buffer_putstr(buffer, ".text\n");
			break;
		case jive_region_section_data:
			jive_buffer_putstr(buffer, ".data\n");
			break;
		case jive_region_section_rodata:
			jive_buffer_putstr(buffer, ".data\n");
			break;
		case jive_region_section_bss:
			jive_buffer_putstr(buffer, ".bss\n");
			break;
	};
	#endif
	if (region->attrs.align > 1) {
		char tmp[80];
		snprintf(tmp, sizeof(tmp), ".align %zd\n", region->attrs.align);
		jive_buffer_putstr(buffer, tmp);
	}
}

static void
emit_labels(jive_seq_point * seq_point, jive_buffer * buffer)
{
	if (seq_point == seq_point->seq_region->first_point)
		emit_region_start_attrs(seq_point->seq_region->region, buffer);
	
	size_t n;
	for(n = 0; n < seq_point->attached_labels.nitems; n++) {
		
		const jive_label_internal * label = seq_point->attached_labels.items[n];
		const char * name = jive_label_internal_get_asmname(label);
		
		if (label->base.flags & jive_label_flags_global) {
			jive_buffer_putstr(buffer, ".globl ");
			jive_buffer_putstr(buffer, name);
			jive_buffer_putstr(buffer, "\n");
		}
		
		jive_buffer_putstr(buffer, name);
		jive_buffer_putstr(buffer,":\n");
	}
}

static void
jive_instruction_generate_assembler(const jive_instruction * instr, jive_buffer * buffer, uint32_t * flags)
{
	const jive_instruction_class * icls = instr->icls;
	if ( (*flags & jive_instruction_encoding_flags_jump_conditional_invert) )
		icls = icls->inverse_jump;
	
	jive_buffer_putstr(buffer, "\t");
	if (icls->write_asm) {
		jive_instruction_encoding_flags iflags;
		iflags = (jive_instruction_encoding_flags) *flags;
		icls->write_asm(icls, buffer, instr->inputs, instr->outputs, instr->immediates, &iflags);
		*flags = (uint32_t) iflags;
	} else {
		jive_buffer_putstr(buffer, "; ");
		jive_buffer_put(buffer, icls->name, strlen(icls->name));
	}
	
	jive_buffer_putstr(buffer, "\n");
}

static void
jive_instruction_node_generate_assembler(jive_seq_node * seq_node, jive_node * node, jive_buffer * buffer)
{
	jive_instruction_node * instr_node = (jive_instruction_node *) node;
	const jive_instruction_class * icls = instr_node->attrs.icls;
	
	const jive_register_name * inregs[icls->ninputs];
	const jive_register_name * outregs[icls->noutputs];
	size_t n;
	for(n=0; n<icls->ninputs; n++) {
		const jive_resource_name * resname = jive_variable_get_resource_name(node->inputs[n]->ssavar->variable);
		inregs[n] = (const jive_register_name *)resname;
	}
	for(n=0; n<icls->noutputs; n++) {
		const jive_resource_name * resname = jive_variable_get_resource_name(node->outputs[n]->ssavar->variable);
		outregs[n] = (const jive_register_name *)resname;
	}
	jive_instruction instr;
	instr.icls = icls;
	instr.inputs = inregs;
	instr.outputs = outregs;
	instr.immediates = instr_node->attrs.immediates;
	
	jive_instruction_generate_assembler(&instr, buffer, &seq_node->flags);
}

static void
jive_bitstring_generate_assembler(const jive_bitstring_type * data_type, jive_output * data_item, jive_buffer * buffer)
{
	const char * prefix = NULL;
	switch(data_type->nbits) {
		case 8:
			prefix = "\t.byte ";
			break;
		case 16:
			prefix = "\t.value ";
			break;
		case 32:
			prefix = "\t.long ";
			break;
		case 64:
			prefix = "\t.quad ";
			break;
	}
	if (!prefix)
		return;
	
	jive_context * context = buffer->context;
	
	char * repr = jive_bitstring_symbolic_expression(context, data_item);
	jive_buffer_putstr(buffer, prefix);
	jive_buffer_putstr(buffer, repr);
	jive_buffer_putstr(buffer, "\n");
	jive_context_free(context, repr);
}

static void
jive_dataitem_generate_assembler(jive_output * data_item, jive_buffer * buffer)
{
	const jive_type * data_type = jive_output_get_type(data_item);
	
	if (data_type->class_ == &JIVE_BITSTRING_TYPE)
		jive_bitstring_generate_assembler((const jive_bitstring_type *) data_type, data_item, buffer);
	else
		jive_context_fatal_error(buffer->context, "Type mismatch: don't know how to generate assembler for type");
}

static void
jive_dataitems_node_generate_assembler(jive_seq_node * seq_node, jive_node * node, jive_buffer * buffer)
{
	size_t n;
	for (n = 0; n < node->ninputs; n++)
		jive_dataitem_generate_assembler(node->inputs[n]->origin, buffer);
}

static void
jive_seq_instruction_generate_assembler(jive_seq_instruction * seq_instr, jive_buffer * buffer)
{
	jive_instruction_generate_assembler(&seq_instr->instr, buffer, &seq_instr->flags);
}

static void
jive_seq_node_generate_assembler(jive_seq_node * seq_node, jive_buffer * buffer)
{
	jive_node * node = seq_node->node;
	if (jive_node_isinstance(node, &JIVE_INSTRUCTION_NODE)) {
		jive_instruction_node_generate_assembler(seq_node, node, buffer);
	} else if (jive_node_isinstance(node, &JIVE_DATAITEMS_NODE)) {
		jive_dataitems_node_generate_assembler(seq_node, node, buffer);
	}
}

void
jive_seq_graph_generate_assembler(jive_seq_graph * seq_graph, jive_buffer * buffer)
{
	jive_seq_point * seq_point;
	JIVE_LIST_ITERATE(seq_graph->points, seq_point, seqpoint_list) {
		emit_labels(seq_point, buffer);
		jive_seq_node * seq_node = jive_seq_node_cast(seq_point);
		if (seq_node) {
			jive_seq_node_generate_assembler(seq_node, buffer);
			continue;
		}
		jive_seq_instruction * seq_instr = jive_seq_instruction_cast(seq_point);
		if (seq_instr) {
			jive_seq_instruction_generate_assembler(seq_instr, buffer);
			continue;
		}
	}
}

void
jive_graph_generate_assembler(jive_graph * graph, jive_buffer * buffer)
{
	jive_seq_graph * seq_graph = jive_graph_sequentialize(graph);
	jive_seq_graph_patch_jumps(seq_graph);
	
	jive_seq_graph_generate_assembler(seq_graph, buffer);
	
	jive_seq_graph_destroy(seq_graph);
}

