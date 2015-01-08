/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
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
#include <jive/util/typeinfo-map.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/seqtype.h>
#include <jive/vsdg/traverser.h>

static inline bool
is_gpr_immediate(jive::output * arg)
{
	return dynamic_cast<const jive::regvalue_op *>(&arg->node()->operation());
}

static void
swap(jive::output ** arg1, jive::output ** arg2)
{
	jive::output * tmp = *arg1;
	*arg1 = *arg2;
	*arg2 = tmp;
}

static void
regvalue_to_immediate(const jive::output * regvalue, jive_immediate * imm)
{
	jive_node * rvnode = regvalue->node();
	JIVE_DEBUG_ASSERT(dynamic_cast<const jive::regvalue_op *>(&rvnode->operation()));
	jive::output * value = rvnode->inputs[1]->origin();

	if (auto bcop = dynamic_cast<const jive::bits::constant_op *>(&value->node()->operation())) {
		jive_immediate_init(imm, jive::bits::value_repr_to_uint(bcop->value()), 0, 0, 0);
		return;
	}

	auto lbop = dynamic_cast<const jive::address::label_to_bitstring_op *>(
		&value->node()->operation());
	if (lbop) {
		jive_immediate_init(imm, 0, lbop->label(), 0, 0);
		return;
	}
	
	JIVE_DEBUG_ASSERT(false);
}

static void
convert_bitbinary(jive_node * node,
	const jive_instruction_class * regreg_icls,
	const jive_instruction_class * regimm_icls)
{
	jive::output * arg1 = node->inputs[0]->origin();
	jive::output * arg2 = node->inputs[1]->origin();
	
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
		jive::output * tmparray0[] = {arg1};
		instr = jive_instruction_node_create_extended(node->region,
			regimm_icls,
			tmparray0, imm);
	} else {
		jive::output * tmparray1[] = {arg1, arg2};
		instr = jive_instruction_node_create(node->region,
			regreg_icls,
			tmparray1, NULL);
	}
	
	jive_output_replace(node->outputs[0], instr->outputs[0]);
}

static void
convert_divmod(jive_node * node, bool sign, size_t index)
{
	jive::output * arg1 = node->inputs[0]->origin();
	jive::output * arg2 = node->inputs[1]->origin();
	
	jive::output * ext;
	const jive_instruction_class * icls;
	if (sign) {
		jive_immediate imm[1];
		jive_immediate_init(&imm[0], 31, 0, 0, 0);
		jive::output * tmparray2[] = {arg1};
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
		
		jive_node * sub = jive_region_get_subroutine_node(node->region);
		jive_node * enter = sub->producer(0)->region->top;
		jive_node_add_input(tmp, &jive::seq::seqtype, enter->outputs[0]);
		
		ext = tmp->outputs[0];
		icls = &jive_i386_instr_int_udiv;
	}
	jive::output * tmparray3[] = {ext, arg1, arg2};
	
	jive_node * instr = jive_instruction_node_create_extended(node->region,
		icls, tmparray3, NULL);
	
	jive_output_replace(node->outputs[0], instr->outputs[index]);
}

static void
convert_complex_bitbinary(jive_node * node,
	const jive_instruction_class * icls,
	size_t result_index)
{
	jive::output * arg1 = node->inputs[0]->origin();
	jive::output * arg2 = node->inputs[1]->origin();
	jive::output * tmparray4[] = {arg1, arg2};
	
	jive_node * instr = jive_instruction_node_create(node->region,
		icls,
		tmparray4, NULL);
	
	jive_output_replace(node->outputs[0], instr->outputs[result_index]);
}

static const jive::detail::typeinfo_map<
	std::function<void(jive_node*)>
> bitbinary_map = {
	{
		&typeid(jive::bits::and_op),
		std::bind(convert_bitbinary,
			std::placeholders::_1,
			&jive_i386_instr_int_and,
			&jive_i386_instr_int_and_immediate)
	},
	{
		&typeid(jive::bits::or_op),
		std::bind(convert_bitbinary,
			std::placeholders::_1,
			&jive_i386_instr_int_or,
			&jive_i386_instr_int_or_immediate)
	},
	{
		&typeid(jive::bits::xor_op),
		std::bind(convert_bitbinary,
			std::placeholders::_1,
			&jive_i386_instr_int_xor,
			&jive_i386_instr_int_xor_immediate)
	},
	{
		&typeid(jive::bits::add_op),
		std::bind(convert_bitbinary,
			std::placeholders::_1,
			&jive_i386_instr_int_add,
			&jive_i386_instr_int_add_immediate)
	},
	{
		&typeid(jive::bits::sub_op),
		std::bind(convert_bitbinary,
			std::placeholders::_1,
			&jive_i386_instr_int_sub,
			&jive_i386_instr_int_sub_immediate)
	},
	{
		&typeid(jive::bits::mul_op),
		std::bind(convert_bitbinary,
			std::placeholders::_1,
			&jive_i386_instr_int_mul,
			&jive_i386_instr_int_mul_immediate)
	},
	{
		&typeid(jive::bits::xor_op),
		std::bind(convert_bitbinary,
			std::placeholders::_1,
			&jive_i386_instr_int_xor,
			&jive_i386_instr_int_xor_immediate)
	},
	{
		&typeid(jive::bits::shr_op),
		std::bind(convert_bitbinary,
			std::placeholders::_1,
			&jive_i386_instr_int_shr,
			&jive_i386_instr_int_shr_immediate)
	},
	{
		&typeid(jive::bits::ashr_op),
		std::bind(convert_bitbinary,
			std::placeholders::_1,
			&jive_i386_instr_int_ashr,
			&jive_i386_instr_int_ashr_immediate)
	},
	{
		&typeid(jive::bits::shl_op),
		std::bind(convert_bitbinary,
			std::placeholders::_1,
			&jive_i386_instr_int_shl,
			&jive_i386_instr_int_shl_immediate)
	},
	{
		&typeid(jive::bits::umulh_op),
		std::bind(convert_complex_bitbinary,
			std::placeholders::_1,
			&jive_i386_instr_int_mul_expand_unsigned,
			0)
	},
	{
		&typeid(jive::bits::smulh_op),
		std::bind(convert_complex_bitbinary,
			std::placeholders::_1,
			&jive_i386_instr_int_mul_expand_signed,
			0)
	},
	{
		&typeid(jive::bits::udiv_op),
		std::bind(convert_divmod,
			std::placeholders::_1,
			false,
			1)
	},
	{
		&typeid(jive::bits::sdiv_op),
		std::bind(convert_divmod,
			std::placeholders::_1,
			true,
			1)
	},
	{
		&typeid(jive::bits::umod_op),
		std::bind(convert_divmod,
			std::placeholders::_1,
			false,
			0)
	},
	{
		&typeid(jive::bits::smod_op),
		std::bind(convert_divmod,
			std::placeholders::_1,
			true,
			0)
	}
};

static void
match_gpr_bitbinary(jive_node * node)
{
	auto i = bitbinary_map.find(&typeid(node->operation()));
	if (i != bitbinary_map.end()) {
		i->second(node);
	} else {
		throw std::logic_error("Unknown operator");
	}
}

static void
convert_bitcmp(
	jive_node * node,
	const jive_instruction_class * jump_icls,
	const jive_instruction_class * inv_jump_icls)
{
	jive::output * arg1 = node->inputs[0]->origin();
	jive::output * arg2 = node->inputs[1]->origin();
	
	bool second_is_immediate = is_gpr_immediate(arg2);
	if (!second_is_immediate) {
		bool first_is_immediate = is_gpr_immediate(arg1);
		if (first_is_immediate) {
			jive::output * tmp = arg1;
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
		jive::output * tmparray5[] = {arg1};
		cmp_instr = jive_instruction_node_create_extended(node->region,
			&jive_i386_instr_int_cmp_immediate,
			tmparray5, imm);
	} else {
		jive::output * tmparray6[] = {arg1, arg2};
		cmp_instr = jive_instruction_node_create(node->region,
			&jive_i386_instr_int_cmp,
			tmparray6, NULL);
	}
	
	jive::ctl::type ctltype;
	jive_immediate imm;
	jive_immediate_init(&imm, 0, 0, 0, 0);
	jive_node * jump_instr = jive_instruction_node_create(node->region, jump_icls,
		{cmp_instr->outputs[0]}, {imm}, {}, {}, {&ctltype});
	jive_output_replace(node->outputs[0], jump_instr->outputs[0]);
}

static const jive::detail::typeinfo_map<
	std::pair<
		const jive_instruction_class *,
		const jive_instruction_class *
	>
> bitcompare_map = {
	{
		&typeid(jive::bits::eq_op),
		{
			&jive_i386_instr_int_jump_equal,
			&jive_i386_instr_int_jump_equal
		}
	},
	{
		&typeid(jive::bits::ne_op),
		{
			&jive_i386_instr_int_jump_notequal,
			&jive_i386_instr_int_jump_notequal
		}
	},
	{
		&typeid(jive::bits::slt_op),
		{
			&jive_i386_instr_int_jump_sless,
			&jive_i386_instr_int_jump_sgreater
		}
	},
	{
		&typeid(jive::bits::sle_op),
		{
			&jive_i386_instr_int_jump_slesseq,
			&jive_i386_instr_int_jump_sgreatereq
		}
	},
	{
		&typeid(jive::bits::sgt_op),
		{
			&jive_i386_instr_int_jump_sgreater,
			&jive_i386_instr_int_jump_sless
		}
	},
	{
		&typeid(jive::bits::sge_op),
		{
			&jive_i386_instr_int_jump_sgreatereq,
			&jive_i386_instr_int_jump_slesseq
		}
	},
	{
		&typeid(jive::bits::ult_op),
		{
			&jive_i386_instr_int_jump_uless,
			&jive_i386_instr_int_jump_ugreater
		}
	},
	{
		&typeid(jive::bits::ule_op),
		{
			&jive_i386_instr_int_jump_ulesseq,
			&jive_i386_instr_int_jump_ugreatereq
		}
	},
	{
		&typeid(jive::bits::ugt_op),
		{
			&jive_i386_instr_int_jump_ugreater,
			&jive_i386_instr_int_jump_uless
		}
	},
	{
		&typeid(jive::bits::uge_op),
		{
			&jive_i386_instr_int_jump_ugreatereq,
			&jive_i386_instr_int_jump_ulesseq
		}
	}
};

static void
match_gpr_bitcmp(jive_node * node)
{
	auto i = bitcompare_map.find(&typeid(node->operation()));
	if (i != bitcompare_map.end()) {
		convert_bitcmp(node, i->second.first, i->second.second);
	} else {
		throw std::logic_error("Unknown operator");
	}
}

static const jive::detail::typeinfo_map<
	const jive_instruction_class *
> bitunary_map = {
	{
		&typeid(jive::bits::neg_op),
		&jive_i386_instr_int_neg
	},
	{
		&typeid(jive::bits::not_op),
		&jive_i386_instr_int_not
	}
};

static void
match_gpr_bitunary(jive_node * node)
{
	auto i = bitunary_map.find(&typeid(node->operation()));
	if (i != bitunary_map.end()) {
		const jive_instruction_class * icls = i->second;
		jive::output * tmparray8[] = {node->inputs[0]->origin()};
		jive_node * instr = jive_instruction_node_create(node->region,
			icls,
			tmparray8, NULL);
		
		jive_output_replace(node->outputs[0], instr->outputs[0]);
	} else {
		throw std::logic_error("Unknown operator");
	}
}

static void
match_gpr_immediate(jive_node * node)
{
	JIVE_DEBUG_ASSERT(dynamic_cast<const jive::regvalue_op *>(&node->operation()));
	
	jive_immediate imm[1];
	regvalue_to_immediate(node->outputs[0], &imm[0]);
	
	jive_node * instr = jive_instruction_node_create_extended(node->region,
		&jive_i386_instr_int_load_imm,
		NULL, imm);
	jive_node_add_input(instr, &jive::seq::seqtype, node->inputs[0]->origin());
	
	jive_output_replace(node->outputs[0], instr->outputs[0]);
}

typedef enum {
	jive_i386_addr_mode_disp
} jive_i386_addr_mode;

typedef struct jive_i386_addr_info {
	jive_i386_addr_mode mode;
	union {
		struct {
			jive::output * base;
			int32_t offset;
		} disp;
	} info;
} jive_i386_addr_info;

static jive_i386_addr_info
classify_address(jive::output * output)
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
	jive_i386_addr_info info = classify_address(node->inputs[0]->origin());
	
	jive_node * instr;
	jive::output * value;
	switch (info.mode) {
		case jive_i386_addr_mode_disp: {
			jive_immediate imm[1];
			jive_immediate_init(&imm[0], info.info.disp.offset, 0, 0, 0);
			jive::output * tmparray9[] = {info.info.disp.base};
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
		jive::input * input = node->inputs[n];
		if (input->gate)
			jive_node_gate_input(instr, input->gate, input->origin());
		else
			jive_node_add_input(instr, &input->type(), input->origin());
	}
	for (n = 1; n < node->noutputs; n++) {
		jive::output * output = node->outputs[n];
		jive::output * rep;
		if (output->gate)
			rep = jive_node_gate_output(instr, output->gate);
		else
			rep = jive_node_add_output(instr, &output->type());
		jive_output_replace(output, rep);
	}
}

static void
match_gpr_store(jive_node * node)
{
	jive_i386_addr_info info = classify_address(node->inputs[0]->origin());
	jive::output * value = node->inputs[1]->origin();
	
	jive_node * instr;
	switch (info.mode) {
		case jive_i386_addr_mode_disp: {
			jive_immediate imm[1];
			jive_immediate_init(&imm[0], info.info.disp.offset, 0, 0, 0);
			jive::output * tmparray10[] = {info.info.disp.base, value};
			instr = jive_instruction_node_create_extended(node->region,
				&jive_i386_instr_int_store32_disp,
				tmparray10, imm);
			break;
		}
	}
	
	size_t n;
	for (n = 2; n < node->ninputs; n++) {
		jive::input * input = node->inputs[n];
		if (input->gate)
			jive_node_gate_input(instr, input->gate, input->origin());
		else
			jive_node_add_input(instr, &input->type(), input->origin());
	}
	for (n = 0; n < node->noutputs; n++) {
		jive::output * output = node->outputs[n];
		jive::output * rep;
		if (output->gate)
			rep = jive_node_gate_output(instr, output->gate);
		else
			rep = jive_node_add_output(instr, &output->type());
		jive_output_replace(output, rep);
	}
}

static void
match_single(jive_node * node, const jive_regselector * regselector)
{
	if (dynamic_cast<const jive::bits::binary_op *>(&node->operation())) {
		const jive_register_class * regcls = jive_regselector_map_output(regselector, node->outputs[0]);
		if (regcls == &jive_i386_regcls_gpr) {
			match_gpr_bitbinary(node);
		} else {
			JIVE_DEBUG_ASSERT(false);
		}
	} else if (dynamic_cast<const jive::bits::unary_op *>(&node->operation())) {
		const jive_register_class * regcls = jive_regselector_map_output(regselector, node->outputs[0]);
		if (regcls == &jive_i386_regcls_gpr) {
			match_gpr_bitunary(node);
		} else {
			JIVE_DEBUG_ASSERT(false);
		}
	} else if (dynamic_cast<const jive::bits::compare_op *>(&node->operation())) {
		const jive_register_class * regcls = jive_regselector_map_input(regselector, node->inputs[0]);
		if (true || (regcls == &jive_i386_regcls_gpr)) {
			match_gpr_bitcmp(node);
		} else {
			JIVE_DEBUG_ASSERT(false);
		}
	} else if (dynamic_cast<const jive::regvalue_op *>(&node->operation())) {
		const jive_register_class * regcls = jive_regselector_map_output(regselector, node->outputs[0]);
		if (regcls == &jive_i386_regcls_gpr) {
			match_gpr_immediate(node);
		} else {
			JIVE_DEBUG_ASSERT(false);
		}
	} else if (dynamic_cast<const jive::load_op *>(&node->operation())) {
		const jive_register_class * regcls = jive_regselector_map_output(regselector, node->outputs[0]);
		if (regcls == &jive_i386_regcls_gpr) {
			match_gpr_load(node);
		} else {
			JIVE_DEBUG_ASSERT(false);
		}
	} else if (dynamic_cast<const jive::store_op *>(&node->operation())) {
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

