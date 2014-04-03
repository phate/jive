/*
 * Copyright 2010 2011 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/backend/i386/instrmatch.h>

#include <jive/arch/address.h>
#include <jive/arch/instruction.h>
#include <jive/arch/load.h>
#include <jive/arch/regvalue.h>
#include <jive/arch/store.h>
#include <jive/arch/subroutine.h>
#include <jive/arch/subroutine/nodes.h>
#include <jive/backend/i386/classifier.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/registerset.h>
#include <jive/types/bitstring.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/traverser.h>

static inline bool
is_gpr_immediate(jive_output * arg)
{
	return jive_node_isinstance(arg->node, &JIVE_REGVALUE_NODE);
}

static void
swap(jive_output ** arg1, jive_output ** arg2)
{
	jive_output * tmp = *arg1;
	*arg1 = *arg2;
	*arg2 = tmp;
}

static void
regvalue_to_immediate(const jive_output * regvalue, jive_immediate * imm)
{
	jive_node * rvnode = regvalue->node;
	JIVE_DEBUG_ASSERT(jive_node_isinstance(rvnode, &JIVE_REGVALUE_NODE));
	jive_output * value = rvnode->inputs[1]->origin;
	
	jive_bitconstant_node * bcnode = jive_bitconstant_node_cast(value->node);
	if (bcnode) {
		jive_immediate_init(imm, jive_bitconstant_node_to_unsigned(bcnode), 0, 0, 0);
		return;
	}
	
	jive_label_to_bitstring_node * lbnode = jive_label_to_bitstring_node_cast(value->node);
	if (lbnode) {
		jive_immediate_init(imm, 0, lbnode->attrs.label, 0, 0);
		return;
	}
	
	JIVE_DEBUG_ASSERT(false);
}

static void
convert_bitbinary(jive_node * node,
	const jive_instruction_class * regreg_icls,
	const jive_instruction_class * regimm_icls)
{
	jive_output * arg1 = node->inputs[0]->origin;
	jive_output * arg2 = node->inputs[1]->origin;
	
	bool commutative = (regreg_icls->flags & jive_instruction_commutative) != 0;
	bool second_is_immediate = false;
	if (commutative && is_gpr_immediate(arg1)) {
		swap(&arg1, &arg2);
		second_is_immediate = true;
	} else if (is_gpr_immediate(arg2)) {
		second_is_immediate = true;
	}
	
	jive_node * instr;
	
	if (second_is_immediate) {
		jive_immediate imm[1];
		regvalue_to_immediate(arg2, &imm[0]);
		jive_output * tmparray0[] = {arg1};
		instr = jive_instruction_node_create_extended(node->region,
			regimm_icls,
			tmparray0, imm);
	} else {
		jive_output * tmparray1[] = {arg1, arg2};
		instr = jive_instruction_node_create(node->region,
			regreg_icls,
			tmparray1, NULL);
	}
	
	jive_output_replace(node->outputs[0], instr->outputs[0]);
}

static void
convert_divmod(jive_node * node, bool sign, size_t index)
{
	jive_output * arg1 = node->inputs[0]->origin;
	jive_output * arg2 = node->inputs[1]->origin;
	
	jive_output * ext;
	const jive_instruction_class * icls;
	if (sign) {
		jive_immediate imm[1];
		jive_immediate_init(&imm[0], 31, 0, 0, 0);
		jive_output * tmparray2[] = {arg1};
		jive_node * tmp = jive_instruction_node_create_extended(node->region,
			&jive_i386_instr_int_ashr_immediate,
			tmparray2, imm);
		
		ext = tmp->outputs[0];
		icls = &jive_i386_instr_int_sdiv;
	} else {
		jive_immediate imm[1];
		jive_immediate_init(&imm[0], 0, 0, 0, 0);
		jive_node * tmp = jive_instruction_node_create_extended(node->region,
			&jive_i386_instr_int_load_imm,
			NULL, imm);
		
		jive_subroutine_node * sub = jive_region_get_subroutine_node(node->region);
		jive_node * enter = sub->base.inputs[0]->origin->node->region->top;
		JIVE_DECLARE_CONTROL_TYPE(ctl);
		jive_node_add_input(tmp, ctl, enter->outputs[0]);
		
		ext = tmp->outputs[0];
		icls = &jive_i386_instr_int_udiv;
	}
	jive_output * tmparray3[] = {ext, arg1, arg2};
	
	jive_node * instr = jive_instruction_node_create_extended(node->region,
		icls, tmparray3, NULL);
	
	jive_output_replace(node->outputs[0], instr->outputs[index]);
}

static void
convert_complex_bitbinary(jive_node * node,
	const jive_instruction_class * icls,
	size_t result_index)
{
	jive_output * arg1 = node->inputs[0]->origin;
	jive_output * arg2 = node->inputs[1]->origin;
	jive_output * tmparray4[] = {arg1, arg2};
	
	jive_node * instr = jive_instruction_node_create(node->region,
		icls,
		tmparray4, NULL);
	
	jive_output_replace(node->outputs[0], instr->outputs[result_index]);
}

static void
match_gpr_bitbinary(jive_node * node)
{
	const jive_bitbinary_operation_class * cls =
		(const jive_bitbinary_operation_class *) node->class_;
	switch (cls->type) {
		case jive_bitop_code_and:
			convert_bitbinary(node,
				&jive_i386_instr_int_and,
				&jive_i386_instr_int_and_immediate);
			return;
		case jive_bitop_code_or:
			convert_bitbinary(node,
				&jive_i386_instr_int_or,
				&jive_i386_instr_int_or_immediate);
			return;
		case jive_bitop_code_xor:
			convert_bitbinary(node,
				&jive_i386_instr_int_xor,
				&jive_i386_instr_int_xor_immediate);
			return;
		case jive_bitop_code_sum:
			convert_bitbinary(node,
				&jive_i386_instr_int_add,
				&jive_i386_instr_int_add_immediate);
			return;
		case jive_bitop_code_difference:
			convert_bitbinary(node,
				&jive_i386_instr_int_sub,
				&jive_i386_instr_int_sub_immediate);
			return;
		case jive_bitop_code_product:
			convert_bitbinary(node,
				&jive_i386_instr_int_mul,
				&jive_i386_instr_int_mul_immediate);
			return;
		case jive_bitop_code_uhiproduct:
			convert_complex_bitbinary(node,
				&jive_i386_instr_int_mul_expand_unsigned,
				0);
			break;
		case jive_bitop_code_shiproduct:
			convert_complex_bitbinary(node,
				&jive_i386_instr_int_mul_expand_signed,
				0);
			break;
		case jive_bitop_code_uquotient:
			convert_divmod(node, false, 1);
			break;
		case jive_bitop_code_squotient:
			convert_divmod(node, true, 1);
			break;
		case jive_bitop_code_umod:
			convert_divmod(node, false, 0);
			break;
		case jive_bitop_code_smod:
			convert_divmod(node, true, 0);
			break;
		case jive_bitop_code_shl:
			convert_bitbinary(node,
				&jive_i386_instr_int_shl,
				&jive_i386_instr_int_shl_immediate);
			return;
		case jive_bitop_code_shr:
			convert_bitbinary(node,
				&jive_i386_instr_int_shr,
				&jive_i386_instr_int_shr_immediate);
			return;
		case jive_bitop_code_ashr:
			convert_bitbinary(node,
				&jive_i386_instr_int_ashr,
				&jive_i386_instr_int_ashr_immediate);
			return;
		case jive_bitop_code_negate:
		case jive_bitop_code_not:
		case jive_bitop_code_invalid:
			JIVE_DEBUG_ASSERT(false);
	}
}

static void
convert_bitcmp(jive_node * node, const jive_instruction_class * jump_icls, const jive_instruction_class * inv_jump_icls)
{
	jive_output * arg1 = node->inputs[0]->origin;
	jive_output * arg2 = node->inputs[1]->origin;
	
	bool second_is_immediate = is_gpr_immediate(arg2);
	if (!second_is_immediate) {
		bool first_is_immediate = is_gpr_immediate(arg1);
		if (first_is_immediate) {
			jive_output * tmp = arg1;
			arg1 = arg2;
			arg2 = tmp;
			jump_icls = inv_jump_icls;
			second_is_immediate = first_is_immediate;
		}
	}
	
	jive_node * cmp_instr;
	
	if (second_is_immediate) {
		jive_immediate imm[1];
		regvalue_to_immediate(arg2, &imm[0]);
		jive_output * tmparray5[] = {arg1};
		cmp_instr = jive_instruction_node_create_extended(node->region,
			&jive_i386_instr_int_cmp_immediate,
			tmparray5, imm);
	} else {
		jive_output * tmparray6[] = {arg1, arg2};
		cmp_instr = jive_instruction_node_create(node->region,
			&jive_i386_instr_int_cmp,
			tmparray6, NULL);
	}
	
	jive_immediate imm[1];
	jive_immediate_init(&imm[0], 0, 0, 0, 0);
	jive_output * tmparray7[] = {cmp_instr->outputs[0]};
	jive_node * jump_instr = jive_instruction_node_create_extended(node->region,
		jump_icls,
		tmparray7,
		imm);
	jive_output_replace(node->outputs[0], jump_instr->outputs[0]);
}

static void
match_gpr_bitcmp(jive_node * node)
{
	const jive_bitcomparison_operation_class * cls =
		(const jive_bitcomparison_operation_class *) node->class_;
	switch (cls->type) {
		case jive_bitcmp_code_equal:
			convert_bitcmp(node,
				&jive_i386_instr_int_jump_equal,
				&jive_i386_instr_int_jump_equal);
			break;
		case jive_bitcmp_code_notequal:
			convert_bitcmp(node,
				&jive_i386_instr_int_jump_notequal,
				&jive_i386_instr_int_jump_notequal);
			break;
		case jive_bitcmp_code_sless:
			convert_bitcmp(node,
				&jive_i386_instr_int_jump_sless,
				&jive_i386_instr_int_jump_sgreater);
			break;
		case jive_bitcmp_code_uless:
			convert_bitcmp(node,
				&jive_i386_instr_int_jump_uless,
				&jive_i386_instr_int_jump_ugreater);
			break;
		case jive_bitcmp_code_slesseq:
			convert_bitcmp(node,
				&jive_i386_instr_int_jump_slesseq,
				&jive_i386_instr_int_jump_sgreatereq);
			break;
		case jive_bitcmp_code_ulesseq:
			convert_bitcmp(node,
				&jive_i386_instr_int_jump_ulesseq,
				&jive_i386_instr_int_jump_ugreatereq);
			break;
		case jive_bitcmp_code_sgreater:
			convert_bitcmp(node,
				&jive_i386_instr_int_jump_sgreater,
				&jive_i386_instr_int_jump_sless);
			break;
		case jive_bitcmp_code_ugreater:
			convert_bitcmp(node,
				&jive_i386_instr_int_jump_ugreater,
				&jive_i386_instr_int_jump_uless);
			break;
		case jive_bitcmp_code_sgreatereq:
			convert_bitcmp(node,
				&jive_i386_instr_int_jump_sgreatereq,
				&jive_i386_instr_int_jump_slesseq);
			break;
		case jive_bitcmp_code_ugreatereq:
			convert_bitcmp(node,
				&jive_i386_instr_int_jump_ugreatereq,
				&jive_i386_instr_int_jump_ulesseq);
			break;
		case jive_bitcmp_code_invalid:
			JIVE_DEBUG_ASSERT(false);
	}
}

static void
match_gpr_bitunary(jive_node * node)
{
	const jive_bitunary_operation_class * cls =
		(const jive_bitunary_operation_class *) node->class_;
	const jive_instruction_class * icls = 0;
	switch (cls->type) {
		case jive_bitop_code_and:
		case jive_bitop_code_or:
		case jive_bitop_code_xor:
		case jive_bitop_code_sum:
		case jive_bitop_code_difference:
		case jive_bitop_code_product:
		case jive_bitop_code_uhiproduct:
		case jive_bitop_code_shiproduct:
		case jive_bitop_code_uquotient:
		case jive_bitop_code_squotient:
		case jive_bitop_code_umod:
		case jive_bitop_code_smod:
		case jive_bitop_code_shl:
		case jive_bitop_code_shr:
		case jive_bitop_code_ashr:
			JIVE_DEBUG_ASSERT(false);
		case jive_bitop_code_negate:
			icls = &jive_i386_instr_int_neg;
			break;
		case jive_bitop_code_not:
			icls = &jive_i386_instr_int_not;
			break;
		case jive_bitop_code_invalid:
			JIVE_DEBUG_ASSERT(false);
	}
	jive_output * tmparray8[] = {node->inputs[0]->origin};
	jive_node * instr = jive_instruction_node_create(node->region,
		icls,
		tmparray8, NULL);
	
	jive_output_replace(node->outputs[0], instr->outputs[0]);
}

static void
match_gpr_immediate(jive_node * node)
{
	JIVE_DEBUG_ASSERT(jive_node_isinstance(node, &JIVE_REGVALUE_NODE));
	
	jive_immediate imm[1];
	regvalue_to_immediate(node->outputs[0], &imm[0]);
	
	jive_node * instr = jive_instruction_node_create_extended(node->region,
		&jive_i386_instr_int_load_imm,
		NULL, imm);
	JIVE_DECLARE_CONTROL_TYPE(ctl);
	jive_node_add_input(instr, ctl, node->inputs[0]->origin);
	
	jive_output_replace(node->outputs[0], instr->outputs[0]);
}

typedef enum {
	jive_i386_addr_mode_disp
} jive_i386_addr_mode;

typedef struct jive_i386_addr_info {
	jive_i386_addr_mode mode;
	union {
		struct {
			jive_output * base;
			int32_t offset;
		} disp;
	} info;
} jive_i386_addr_info;

static jive_i386_addr_info
classify_address(jive_output * output)
{
	jive_i386_addr_info info;
	info.mode = jive_i386_addr_mode_disp;
	info.info.disp.base = output;
	info.info.disp.offset = 0;
	return info;
}

static void
match_gpr_load(jive_node * node)
{
	jive_i386_addr_info info = classify_address(node->inputs[0]->origin);
	
	jive_node * instr;
	jive_output * value;
	switch (info.mode) {
		case jive_i386_addr_mode_disp: {
			jive_immediate imm[1];
			jive_immediate_init(&imm[0], info.info.disp.offset, 0, 0, 0);
			jive_output * tmparray9[] = {info.info.disp.base};
			instr = jive_instruction_node_create_extended(node->region,
				&jive_i386_instr_int_load32_disp,
				tmparray9, imm);
			value = instr->outputs[0];
			break;
		}
	}
	
	jive_output_replace(node->outputs[0], value);
	size_t n;
	for (n = 1; n < node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		if (input->gate)
			jive_node_gate_input(instr, input->gate, input->origin);
		else
			jive_node_add_input(instr, jive_input_get_type(input), input->origin);
	}
	for (n = 1; n < node->noutputs; n++) {
		jive_output * output = node->outputs[n];
		jive_output * rep;
		if (output->gate)
			rep = jive_node_gate_output(instr, output->gate);
		else
			rep = jive_node_add_output(instr, jive_output_get_type(output));
		jive_output_replace(output, rep);
	}
}

static void
match_gpr_store(jive_node * node)
{
	jive_i386_addr_info info = classify_address(node->inputs[0]->origin);
	jive_output * value = node->inputs[1]->origin;
	
	jive_node * instr;
	switch (info.mode) {
		case jive_i386_addr_mode_disp: {
			jive_immediate imm[1];
			jive_immediate_init(&imm[0], info.info.disp.offset, 0, 0, 0);
			jive_output * tmparray10[] = {info.info.disp.base, value};
			instr = jive_instruction_node_create_extended(node->region,
				&jive_i386_instr_int_store32_disp,
				tmparray10, imm);
			break;
		}
	}
	
	size_t n;
	for (n = 2; n < node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		if (input->gate)
			jive_node_gate_input(instr, input->gate, input->origin);
		else
			jive_node_add_input(instr, jive_input_get_type(input), input->origin);
	}
	for (n = 0; n < node->noutputs; n++) {
		jive_output * output = node->outputs[n];
		jive_output * rep;
		if (output->gate)
			rep = jive_node_gate_output(instr, output->gate);
		else
			rep = jive_node_add_output(instr, jive_output_get_type(output));
		jive_output_replace(output, rep);
	}
}

static void
match_single(jive_node * node, const jive_regselector * regselector)
{
	if (jive_node_isinstance(node, &JIVE_BITBINARY_NODE)) {
		const jive_register_class * regcls = jive_regselector_map_output(regselector, node->outputs[0]);
		if (regcls == &jive_i386_regcls_gpr) {
			match_gpr_bitbinary(node);
		} else {
			JIVE_DEBUG_ASSERT(false);
		}
	} else if (jive_node_isinstance(node, &JIVE_BITUNARY_NODE)) {
		const jive_register_class * regcls = jive_regselector_map_output(regselector, node->outputs[0]);
		if (regcls == &jive_i386_regcls_gpr) {
			match_gpr_bitunary(node);
		} else {
			JIVE_DEBUG_ASSERT(false);
		}
	} else if (jive_node_isinstance(node, &JIVE_BITCOMPARISON_NODE)) {
		const jive_register_class * regcls = jive_regselector_map_input(regselector, node->inputs[0]);
		if (true || (regcls == &jive_i386_regcls_gpr)) {
			match_gpr_bitcmp(node);
		} else {
			JIVE_DEBUG_ASSERT(false);
		}
	} else if (jive_node_isinstance(node, &JIVE_REGVALUE_NODE)) {
		const jive_register_class * regcls = jive_regselector_map_output(regselector, node->outputs[0]);
		if (regcls == &jive_i386_regcls_gpr) {
			match_gpr_immediate(node);
		} else {
			JIVE_DEBUG_ASSERT(false);
		}
	} else if (jive_node_isinstance(node, &JIVE_LOAD_NODE)) {
		const jive_register_class * regcls = jive_regselector_map_output(regselector, node->outputs[0]);
		if (regcls == &jive_i386_regcls_gpr) {
			match_gpr_load(node);
		} else {
			JIVE_DEBUG_ASSERT(false);
		}
	} else if (jive_node_isinstance(node, &JIVE_STORE_NODE)) {
		const jive_register_class * regcls = jive_regselector_map_input(regselector, node->inputs[1]);
		if (regcls == &jive_i386_regcls_gpr) {
			match_gpr_store(node);
		} else {
			JIVE_DEBUG_ASSERT(false);
		}
	}
}

void
jive_i386_match_instructions(jive_graph * graph, const jive_regselector * regselector)
{
	jive_traverser * trav;
	
	trav = jive_bottomup_traverser_create(graph);
	
	for(;;) {
		jive_node * node = jive_traverser_next(trav);
		if (!node) break;
		match_single(node, regselector);
	}
	
	jive_traverser_destroy(trav);
}

