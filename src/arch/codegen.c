/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/codegen.h>

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include <jive/arch/dataobject.h>
#include <jive/arch/instruction.h>
#include <jive/arch/instructionset.h>
#include <jive/arch/label-mapper.h>
#include <jive/arch/sequence.h>
#include <jive/arch/subroutine.h>
#include <jive/arch/subroutine/nodes.h>
#include <jive/types/bitstring/constant.h>
#include <jive/util/byteswap.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/theta.h>
#include <jive/vsdg/variable.h>

/* map the label to a suitable symref:
- for an internal label, map to section+offset
- for an external label, map to reference to linker symbol_mapper
- otherwise we just have a statically known value */
static void
jive_imm_add_label_resolve(
	jive_codegen_imm * imm,
	const jive_seq_point * current,
	jive_label_symbol_mapper * symbol_mapper,
	const jive_seq_label * label)
{
	if (!label)
		return;
	
	if (label->type == jive_seq_label_type_internal) {
		const jive_seq_point * sp = label->internal;
		jive_stdsectionid section = jive_seq_point_map_to_section(sp);
		imm->info = jive_codegen_imm_info_static_unknown;
		if (sp->address.offset != -1)
			imm->value += sp->address.offset;
		imm->symref = jive_symref_section(section);
	} else if (label->type == jive_seq_label_type_external) {
		imm->info = jive_codegen_imm_info_static_unknown;
		imm->symref = jive_symref_linker_symbol(label->external);
	}
}

/* subtract address of a sequence point from a given immediate */
static void
jive_imm_sub_label_resolve(
	jive_codegen_imm * imm,
	const jive_seq_point * current,
	const jive_seq_label * label)
{
	if (!label || label->type != jive_seq_label_type_internal)
		return;
	
	const jive_seq_point * target = label->internal;
	
	jive_stdsectionid current_section = jive_seq_point_map_to_section(current);
	jive_stdsectionid target_section = jive_seq_point_map_to_section(target);
	
	if (current_section == target_section) {
		if (target->address.offset != -1 && current->address.offset != -1) {
			imm->value += current->address.offset - target->address.offset;
		}
		imm->pc_relative = true;
		imm->info = jive_codegen_imm_info_static_unknown;
	}
}

static jive_codegen_imm
jive_codegen_imm_resolve(
	const jive_seq_imm * self,
	jive_label_symbol_mapper * symbol_mapper,
	const jive_seq_point * for_point)
{
	/* catch the case of taking the difference of two internal labels in
	 * the same section: this can be resolved at compile time */
	
	bool add_internal = (self->add_label.type == jive_seq_label_type_internal);
	bool sub_internal = (self->sub_label.type == jive_seq_label_type_internal);
	
	if (add_internal && sub_internal) {
		jive_seq_point * add_point = self->add_label.internal;
		jive_seq_point * sub_point = self->sub_label.internal;
		jive_stdsectionid add_section = jive_seq_point_map_to_section(
			add_point);
		jive_stdsectionid sub_section = jive_seq_point_map_to_section(
			sub_point);
		if (add_section == sub_section) {
			jive_codegen_imm immval;
			immval.value = add_point->address.offset -
				sub_point->address.offset + self->value;
			if (add_point->address.offset != -1 && sub_point->address.offset != -1)
				immval.info = jive_codegen_imm_info_dynamic_known;
			else
				immval.info = jive_codegen_imm_info_dynamic_unknown;
			immval.symref = jive_symref_none();
			immval.pc_relative = false;
			return immval;
		}
	}
	
	jive_codegen_imm imm;
	imm.value = self->value;
	imm.info = jive_codegen_imm_info_dynamic_known;
	imm.symref = jive_symref_none();
	imm.pc_relative = false;
	
	jive_imm_add_label_resolve(&imm, for_point, symbol_mapper, &self->add_label);
	
	jive_imm_sub_label_resolve(&imm, for_point, &self->sub_label);
	
	return imm;
}

static void
jive_seq_instruction_generate_code(
	jive_seq_instruction * seq_instr,
	jive_label_symbol_mapper * symbol_mapper,
	jive_section * section)
{
	const jive_instruction_class * icls = seq_instr->icls;
	
	if ( (seq_instr->flags & jive_instruction_encoding_flags_jump_conditional_invert) )
		icls = icls->inverse_jump;
	
	jive_codegen_imm immvals[icls->nimmediates];
	size_t n;
	
	for (n = 0; n < icls->nimmediates; ++n) {
		jive_seq_imm imm = seq_instr->imm[n];
		if (n == 0 && (icls->flags & jive_instruction_jump_relative)) {
			imm.sub_label.type = jive_seq_label_type_internal;
			imm.sub_label.internal = &seq_instr->base;
		}
		immvals[n] = jive_codegen_imm_resolve(&imm, symbol_mapper, &seq_instr->base);
	}
	
	icls->encode(icls, section, seq_instr->inputs, seq_instr->outputs, immvals,
		&seq_instr->flags);
}

static void
jive_seq_data_generate_code(
	const jive_seq_data * seq_data,
	jive_section * section)
{
	size_t n;
	for (n = 0; n < seq_data->nitems; ++n) {
		const jive_seq_dataitem * item = &seq_data->items[n];
		switch (item->format) {
			case jive_seq_dataitem_fmt_8: {
				uint8_t value = item->value;
				jive_buffer_put(&section->contents, &value, 1);
				break;
			}
			case jive_seq_dataitem_fmt_le16: {
				uint16_t value = jive_cpu_to_le16(item->value);
				jive_buffer_put(&section->contents, &value, 2);
				break;
			}
			case jive_seq_dataitem_fmt_be16: {
				uint16_t value = jive_cpu_to_be16(item->value);
				jive_buffer_put(&section->contents, &value, 2);
				break;
			}
			case jive_seq_dataitem_fmt_le32: {
				uint32_t value = jive_cpu_to_le32(item->value);
				jive_buffer_put(&section->contents, &value, 4);
				break;
			}
			case jive_seq_dataitem_fmt_be32: {
				uint32_t value = jive_cpu_to_be32(item->value);
				jive_buffer_put(&section->contents, &value, 4);
				break;
			}
			case jive_seq_dataitem_fmt_le64: {
				uint64_t value = jive_cpu_to_le64(item->value);
				jive_buffer_put(&section->contents, &value, 8);
				break;
			}
			case jive_seq_dataitem_fmt_be64: {
				uint64_t value = jive_cpu_to_be64(item->value);
				jive_buffer_put(&section->contents, &value, 8);
				break;
			}
			case jive_seq_dataitem_fmt_none: {
				break;
			}
		}
	}
}

static void
generate_code(
	jive_seq_graph * seq_graph,
	jive_label_symbol_mapper * symbol_mapper,
	jive_compilate * compilate)
{
	jive_seq_point * seq_point;
	
	JIVE_LIST_ITERATE(seq_graph->points, seq_point, seqpoint_list) {
		jive_stdsectionid sectionid =
			jive_seq_point_map_to_section(seq_point->seq_region->first_point);
		jive_section * section = jive_compilate_get_standard_section(
			compilate, sectionid);
		if (!section) continue;

		jive_offset offset = section->contents.data.size();
		if (jive_seq_point_isinstance(seq_point, &JIVE_SEQ_INSTRUCTION)) {
			jive_seq_instruction * seq_instr = (jive_seq_instruction *) seq_point;
			jive_seq_instruction_generate_code(
				seq_instr,
				symbol_mapper,
				section);
		} else if (jive_seq_point_isinstance(seq_point, &JIVE_SEQ_DATA)) {
			jive_seq_data * seq_data = (jive_seq_data *) seq_point;
			jive_seq_data_generate_code(seq_data, section);
		}

		size_t size = section->contents.data.size() - offset;
		if (offset != seq_point->address.offset || size != seq_point->size) {
			jive_address_init(&seq_point->address, sectionid, offset);
			seq_point->size = size;
			seq_point->seq_region->seq_graph->addrs_changed = true;
		}
	}
}

void
jive_seq_graph_generate_code(
	jive_seq_graph * seq_graph,
	jive_label_symbol_mapper * symbol_mapper,
	jive_compilate * buffer)
{
	/* redo until no labels change anymore; this is actually a bit too
	pessimistic, as we only need to redo if
	- a *forward* reference label may have changed AND
	- the encoding of at least one instruction depends on
	  the value of one of the changed labels */
	while (seq_graph->addrs_changed) {
		jive_compilate_clear(buffer);
		seq_graph->addrs_changed = false;
		generate_code(seq_graph, symbol_mapper, buffer);
	}
}

/* for a conditional branch with two possibly different continuation points,
patch in unconditional branch when condition for primary target is not met */
static void
jive_seq_graph_patch_jump_targets(
	jive_seq_graph * seq_graph,
	jive_seq_instruction * seq_instr,
	jive_node * inode,
	const jive::instruction_op & op)
{
	size_t index = op.icls()->noutputs;
	JIVE_DEBUG_ASSERT(inode->noutputs);
	jive::output * ctl_out = inode->outputs[index];
	JIVE_DEBUG_ASSERT(dynamic_cast<jive::ctl::output*>(ctl_out));
	
	JIVE_DEBUG_ASSERT(ctl_out->users.first == ctl_out->users.last);
	if (!ctl_out->users.first)
		return;
	jive_node * user = ctl_out->users.first->node;
	JIVE_DEBUG_ASSERT(user);
	
	jive_seq_point * primary_tgt = 0, * secondary_tgt = 0;
	if (dynamic_cast<const jive::subroutine_tail_op *>(&user->operation())) {
		return;
	} else if (dynamic_cast<const jive::gamma_op *>(&user->operation())) {
		jive_region * primary_region = user->producer(0)->region;
		jive_region * secondary_region = user->producer(1)->region;
		primary_tgt = jive_seq_graph_map_region(seq_graph, primary_region)->first_point;
		secondary_tgt = jive_seq_graph_map_region(seq_graph, secondary_region)->first_point;
	} else {
		JIVE_DEBUG_ASSERT(dynamic_cast<const jive::theta_tail_op *>(&user->operation()));
		jive_region * region = user->region;
		primary_tgt = jive_seq_graph_map_region(seq_graph, region)->first_point;
		secondary_tgt = jive_seq_graph_map_region(seq_graph, region)->last_point;
	}
	
	if ( (op.icls()->flags & jive_instruction_jump_conditional_invertible) ) {
		if (primary_tgt == seq_instr->base.seqpoint_list.next) {
			primary_tgt = secondary_tgt;
			secondary_tgt = 0;
			seq_instr->flags =
				seq_instr->flags |
				jive_instruction_encoding_flags_jump_conditional_invert;
		}
	}
	
	seq_instr->imm[0].value = 0;
	seq_instr->imm[0].add_label.type = jive_seq_label_type_internal;
	seq_instr->imm[0].add_label.internal = primary_tgt;
	
	if (secondary_tgt) {
		const jive_instructionset * isa = jive_region_get_instructionset(inode->region);
		const jive_instruction_class * jump_icls = jive_instructionset_get_jump_instruction_class(isa);
		
		jive_seq_imm imm;
		imm.value = 0;
		imm.add_label.type = jive_seq_label_type_internal;
		imm.add_label.internal = secondary_tgt;
		imm.sub_label.type = jive_seq_label_type_none;
		imm.modifier = NULL;
		
		jive_seq_instruction_create_after(&seq_instr->base, jump_icls,
			NULL, NULL, &imm);
	}
}

/* if a region was taken "out-of-line", patch in a jump to return to the
anchor point */
static void
jive_seq_graph_patch_region_end(jive_seq_graph * seq_graph, jive_seq_point * seq_point)
{
	if (seq_point->seq_region->inlined)
		return;
	
	jive_node * anchor_node = seq_point->node->outputs[0]->users.first->node;
	const jive_instructionset * isa = jive_region_get_instructionset(anchor_node->region);
	const jive_instruction_class * jump_icls = jive_instructionset_get_jump_instruction_class(isa);
	
	jive_seq_imm imm;
	imm.value = 0;
	imm.add_label.type = jive_seq_label_type_internal;
	imm.add_label.internal = jive_seq_graph_map_node(seq_graph, anchor_node);
	imm.sub_label.type = jive_seq_label_type_none;
	imm.modifier = NULL;
	
	jive_seq_instruction_create_after(seq_point, jump_icls,
		NULL, NULL, &imm);
}

void
jive_seq_graph_patch_jumps(jive_seq_graph * seq_graph)
{
	jive_seq_point * seq_point;
	JIVE_LIST_ITERATE(seq_graph->points, seq_point, seqpoint_list) {
		if (!seq_point->node) {
			continue;
		}
		if (jive::graph_tail_operation() == seq_point->node->operation()) {
			continue;
		}

		jive_seq_instruction * seq_instr = jive_seq_instruction_cast(seq_point);
		if (seq_instr && (seq_instr->icls->flags & jive_instruction_jump)) {
			jive_seq_graph_patch_jump_targets(seq_graph, seq_instr, seq_point->node,
				static_cast<const jive::instruction_op &>(seq_point->node->operation()));
		}
		
		if (seq_point->node == seq_point->node->region->bottom)
			jive_seq_graph_patch_region_end(seq_graph, seq_point);
	}
}

void
jive_graph_generate_code(
	jive_graph * graph,
	jive_label_symbol_mapper * symbol_mapper,
	jive_compilate * buffer)
{
	jive_seq_graph * seq_graph = jive_graph_sequentialize(graph);
	jive_seq_graph_patch_jumps(seq_graph);
	
	jive_seq_graph_generate_code(seq_graph, symbol_mapper, buffer);
	
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
emit_labels(
	jive_seq_point * seq_point,
	jive_label_name_mapper * name_mapper,
	jive_buffer * buffer)
{
	if (seq_point == seq_point->seq_region->first_point)
		emit_region_start_attrs(seq_point->seq_region->region, buffer);
	
	if (seq_point->local_symbol) {
		const char * name = jive_label_name_mapper_map_anon_symbol(name_mapper, seq_point);
		jive_buffer_putstr(buffer, name);
		jive_buffer_putstr(buffer,":\n");
	}
	
	size_t n;
	for(n = 0; n < seq_point->named_symbols.size(); n++) {
		const jive_linker_symbol * symbol = seq_point->named_symbols[n];
		const char * name = NULL;
		name = jive_label_name_mapper_map_named_symbol(name_mapper, symbol);
		jive_buffer_putstr(buffer, ".globl ");
		jive_buffer_putstr(buffer, name);
		jive_buffer_putstr(buffer, "\n");
		jive_buffer_putstr(buffer, name);
		jive_buffer_putstr(buffer,":\n");
	}
}

static const char *
jive_asmgen_label_resolve(
	const jive_seq_label * label,
	jive_label_name_mapper * name_mapper)
{
	switch (label->type) {
		case jive_seq_label_type_none: {
			return NULL;
		}
		case jive_seq_label_type_external: {
			return jive_label_name_mapper_map_named_symbol(
				name_mapper, label->external);
		}
		case jive_seq_label_type_internal: {
			return jive_label_name_mapper_map_anon_symbol(
				name_mapper, label->internal);
		}
	}
	
	return NULL;
}

static jive_asmgen_imm
jive_asmgen_imm_resolve(
	const jive_seq_imm * imm,
	jive_label_name_mapper * name_mapper)
{
	jive_asmgen_imm result;
	result.value = imm->value;
	result.add_symbol = jive_asmgen_label_resolve(&imm->add_label, name_mapper);
	result.sub_symbol = jive_asmgen_label_resolve(&imm->sub_label, name_mapper);
	/* FIXME: pass modifier */
	return result;
}

static void
jive_seq_instruction_generate_assembler(
	jive_seq_instruction * seq_instr,
	jive_label_name_mapper * name_mapper,
	jive_buffer * buffer)
{
	const jive_instruction_class * icls = seq_instr->icls;
	if ( (seq_instr->flags & jive_instruction_encoding_flags_jump_conditional_invert) )
		icls = icls->inverse_jump;
	
	jive_buffer_putstr(buffer, "\t");
	if (icls->write_asm) {
		jive_asmgen_imm imm[icls->nimmediates];
		size_t n;
		for (n = 0; n < icls->nimmediates; ++n) {
			imm[n] = jive_asmgen_imm_resolve(&seq_instr->imm[n], name_mapper);
		}
		icls->write_asm(icls, buffer, seq_instr->inputs, seq_instr->outputs, imm,
			&seq_instr->flags);
	} else {
		jive_buffer_putstr(buffer, "; ");
		jive_buffer_put(buffer, icls->name, strlen(icls->name));
	}
	
	jive_buffer_putstr(buffer, "\n");
}

static void
jive_seq_data_generate_assembler(
	const jive_seq_data * seq_data,
	jive_label_name_mapper * name_mapper,
	jive_buffer * buffer)
{
	char tmp[80];
	size_t n;
	for (n = 0; n < seq_data->nitems; ++n) {
		const jive_seq_dataitem * item = &seq_data->items[n];
		uint64_t value = item->value;
		const char * format = NULL;
		switch (item->format) {
			case jive_seq_dataitem_fmt_8:
				format = "\t.byte 0x%" "llx" "\n";
				break;
			case jive_seq_dataitem_fmt_le16:
			case jive_seq_dataitem_fmt_be16:
				format = "\t.value 0x%" "llx" "\n";
				break;
			case jive_seq_dataitem_fmt_le32:
			case jive_seq_dataitem_fmt_be32:
				format = "\t.long 0x%" "llx" "\n";
				break;
			case jive_seq_dataitem_fmt_le64:
			case jive_seq_dataitem_fmt_be64:
				format = "\t.quad 0x%" "llx" "\n";
				break;
			default:
				format = NULL;
				break;
		}
		if (format) {
			snprintf(tmp, sizeof(tmp), format, value);
			jive_buffer_putstr(buffer, tmp);
		}
	}
}

void
jive_seq_graph_generate_assembler(
	jive_seq_graph * seq_graph,
	jive_label_name_mapper * name_mapper,
	jive_buffer * buffer)
{
	jive_seq_point * seq_point;
	JIVE_LIST_ITERATE(seq_graph->points, seq_point, seqpoint_list) {
		emit_labels(seq_point, name_mapper, buffer);
		jive_seq_instruction * seq_instr = jive_seq_instruction_cast(seq_point);
		if (seq_instr) {
			jive_seq_instruction_generate_assembler(seq_instr, name_mapper, buffer);
			continue;
		}
		jive_seq_data * seq_data = jive_seq_data_cast(seq_point);
		if (seq_data) {
			jive_seq_data_generate_assembler(seq_data, name_mapper, buffer);
			continue;
		}
	}
}

void
jive_graph_generate_assembler(
	jive_graph * graph,
	jive_label_name_mapper * mapper,
	jive_buffer * buffer)
{
	jive_seq_graph * seq_graph = jive_graph_sequentialize(graph);
	jive_seq_graph_patch_jumps(seq_graph);
	
	jive_seq_graph_generate_assembler(seq_graph, mapper, buffer);
	
	jive_seq_graph_destroy(seq_graph);
}

