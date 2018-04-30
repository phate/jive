/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/address.h>
#include <jive/arch/instruction.h>
#include <jive/arch/load.h>
#include <jive/arch/regvalue.h>
#include <jive/arch/store.h>
#include <jive/arch/subroutine.h>
#include <jive/arch/subroutine/nodes.h>
#include <jive/backend/i386/classifier.h>
#include <jive/backend/i386/instructionmatch.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/registerset.h>
#include <jive/rvsdg/control.h>
#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/structural-node.h>
#include <jive/rvsdg/traverser.h>
#include <jive/types/bitstring.h>

namespace jive {
namespace i386 {

static void
swap(jive::output ** arg1, jive::output ** arg2)
{
	auto tmp = *arg1;
	*arg1 = *arg2;
	*arg2 = tmp;
}

static jive::immediate
regvalue_to_immediate(const jive::node * node)
{
	JIVE_DEBUG_ASSERT(is_regvalue_node(node));
	auto rvop = static_cast<const jive::regvalue_op*>(&node->operation());

	if (auto op = dynamic_cast<const bitconstant_op*>(&rvop->operation()))
		return op->value().to_uint();

	if (auto op = dynamic_cast<const lbl2bit_op*>(&rvop->operation()))
		return jive::immediate(0, op->label());

	JIVE_ASSERT(0 && "Cannot handle nullary operator.");
}

static void
convert_bitbinary(
	jive::simple_node * node,
	const jive::instruction * regreg,
	const jive::instruction * regimm)
{
	auto arg1 = node->input(0)->origin();
	auto arg2 = node->input(1)->origin();

	bool second_is_immediate = false;
	if (regreg->is_commutative() && is_regvalue_node(arg1->node())) {
		swap(&arg1, &arg2);
		second_is_immediate = true;
	} else if (is_regvalue_node(arg2->node())) {
		second_is_immediate = true;
	}

	jive::node * instr;

	if (second_is_immediate) {
		auto imm = regvalue_to_immediate(arg2->node());
		auto tmp = jive::immediate_op::create(node->region(), imm);
		instr = jive::create_instruction(node->region(), regimm, {arg1, tmp});
	} else {
		instr = jive::create_instruction(node->region(), regreg, {arg1, arg2});
	}

	node->output(0)->divert_users(instr->output(0));
}

static void
convert_divmod(jive::simple_node * node, bool sign, size_t index)
{
	auto arg1 = node->input(0)->origin();
	auto arg2 = node->input(1)->origin();

	jive::output * ext;
	const jive::instruction * icls;
	if (sign) {
		jive::immediate imm(31);
		auto i = jive::immediate_op::create(node->region(), imm);
		auto tmp = jive::create_instruction(node->region(),
			&jive::i386::instr_int_ashr_immediate::instance(), {arg1, i});

		ext = tmp->output(0);
		icls = &jive::i386::instr_int_sdiv::instance();
	} else {
		jive::immediate imm(0);
		auto i = jive::immediate_op::create(node->region(), imm);
		auto tmp = jive::create_instruction(node->region(),
			&jive::i386::instr_int_load_imm::instance(), {i});

		ext = tmp->output(0);
		icls = &jive::i386::instr_int_udiv::instance();
	}

	auto instr = jive::create_instruction(node->region(), icls, {ext, arg1, arg2});
	node->output(0)->divert_users(instr->output(index));
}

static void
convert_complex_bitbinary(
	jive::simple_node * node,
	const jive::instruction * icls,
	size_t result_index)
{
	auto arg1 = node->input(0)->origin();
	auto arg2 = node->input(1)->origin();

	auto instr = jive::create_instruction(node->region(), icls, {arg1, arg2});
	node->output(0)->divert_users(instr->output(result_index));
}

static void
convert_bitcmp(
	jive::node * node_,
	const jive::instruction * jump_icls,
	const jive::instruction * inv_jump_icls)
{
	JIVE_DEBUG_ASSERT(is<match_op>(node_));

	auto node = node_->input(0)->origin()->node();

	auto arg1 = node->input(0)->origin();
	auto arg2 = node->input(1)->origin();

	bool second_is_immediate = is_regvalue_node(arg2->node());
	if (!second_is_immediate) {
		bool first_is_immediate = is_regvalue_node(arg1->node());
		if (first_is_immediate) {
			auto tmp = arg1;
			arg1 = arg2;
			arg2 = tmp;
			jump_icls = inv_jump_icls;
			second_is_immediate = first_is_immediate;
		}
	}

	jive::node * cmp_instr;

	if (second_is_immediate) {
		auto imm = regvalue_to_immediate(arg2->node());
		auto tmp = jive::immediate_op::create(node->region(), imm);
		cmp_instr = jive::create_instruction(node->region(),
			&jive::i386::instr_int_cmp_immediate::instance(), {arg1, tmp});
	} else {
		cmp_instr = jive::create_instruction(node->region(),
			&jive::i386::instr_int_cmp::instance(), {arg1, arg2});
	}

	jive::immediate imm(0);
	auto tmp = jive::immediate_op::create(node->region(), imm);
	auto jump_instr = jive::create_instruction(node->region(), jump_icls,
		{cmp_instr->output(0), tmp}, {}, {jive::ctl2});
	node_->output(0)->divert_users(jump_instr->output(0));
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
convert_bitload_gpr(jive::node * node)
{
	std::vector<jive::port> iports;
	std::vector<jive::port> oports;
	std::vector<jive::output*> operands;

	jive_i386_addr_info info = classify_address(node->input(0)->origin());
	JIVE_DEBUG_ASSERT(info.mode == jive_i386_addr_mode_disp);

	jive::immediate imm(info.info.disp.offset);
	operands.push_back(info.info.disp.base);
	operands.push_back(jive::immediate_op::create(node->region(), imm));

	for (size_t n = 1; n < node->ninputs(); n++) {
		iports.push_back(node->input(n)->port());
		operands.push_back(node->input(n)->origin());
	}

	for (size_t n = 1; n < node->noutputs(); n++)
		oports.push_back(node->output(n)->port());

	auto instr = jive::create_instruction(node->region(),
		&jive::i386::instr_int_load32_disp::instance(), operands, iports, oports);

	divert_users(node, outputs(instr));
}

static void
convert_bitstore_gpr(jive::node * node)
{
	std::vector<jive::port> iports;
	std::vector<jive::port> oports;
	std::vector<jive::output*> operands;

	jive_i386_addr_info info = classify_address(node->input(0)->origin());
	JIVE_DEBUG_ASSERT(info.mode == jive_i386_addr_mode_disp);

	jive::immediate imm(info.info.disp.offset);
	operands.push_back(info.info.disp.base);
	operands.push_back(node->input(1)->origin());
	operands.push_back(jive::immediate_op::create(node->region(), imm));

	for (size_t n = 2; n < node->ninputs(); n++) {
		iports.push_back(node->input(n)->port());
		operands.push_back(node->input(n)->origin());
	}

	for (size_t n = 0; n < node->noutputs(); n++)
		oports.push_back(node->output(n)->port());

	auto instr = jive::create_instruction(node->region(),
		&jive::i386::instr_int_store32_disp::instance(), operands, iports, oports);

	divert_users(node, outputs(instr));
}

static void
match_bitbinary(jive::simple_node * node)
{
	JIVE_DEBUG_ASSERT(is<bitbinary_op>(node));
	JIVE_DEBUG_ASSERT(node->ninputs() == 2);
	auto & op = node->operation();

	static std::unordered_map<std::type_index, std::function<void(simple_node*)>> map({
		{
			typeid(jive::bitadd_op),
			std::bind(convert_bitbinary,
				std::placeholders::_1,
				&jive::i386::instr_int_add::instance(),
				&jive::i386::instr_int_add_immediate::instance())
		},
		{
			typeid(jive::bitand_op),
			std::bind(convert_bitbinary,
				std::placeholders::_1,
				&jive::i386::instr_int_and::instance(),
				&jive::i386::instr_int_and_immediate::instance())
		},
		{
			typeid(jive::bitashr_op),
			std::bind(convert_bitbinary,
				std::placeholders::_1,
				&jive::i386::instr_int_ashr::instance(),
				&jive::i386::instr_int_ashr_immediate::instance())
		},
		{
			typeid(jive::bitmul_op),
			std::bind(convert_bitbinary,
				std::placeholders::_1,
				&jive::i386::instr_int_mul::instance(),
				&jive::i386::instr_int_mul_immediate::instance())
		},
		{
			typeid(jive::bitor_op),
			std::bind(convert_bitbinary,
				std::placeholders::_1,
				&jive::i386::instr_int_or::instance(),
				&jive::i386::instr_int_or_immediate::instance())
		},
		{typeid(jive::bitsdiv_op), std::bind(convert_divmod, std::placeholders::_1, true, 1)},
		{
			typeid(jive::bitshl_op),
			std::bind(convert_bitbinary,
				std::placeholders::_1,
				&jive::i386::instr_int_shl::instance(),
				&jive::i386::instr_int_shl_immediate::instance())
		},
		{
			typeid(jive::bitshr_op),
			std::bind(convert_bitbinary,
				std::placeholders::_1,
				&jive::i386::instr_int_shr::instance(),
				&jive::i386::instr_int_shr_immediate::instance())
		},
		{typeid(jive::bitsmod_op), std::bind(convert_divmod, std::placeholders::_1, true, 0)},
		{
			typeid(jive::bitsmulh_op),
			std::bind(convert_complex_bitbinary,
				std::placeholders::_1,
				&jive::i386::instr_int_mul_expand_signed::instance(), 0)
		},
		{
			typeid(jive::bitsub_op),
			std::bind(convert_bitbinary,
				std::placeholders::_1,
				&jive::i386::instr_int_sub::instance(),
				&jive::i386::instr_int_sub_immediate::instance())
		},
		{typeid(jive::bitudiv_op), std::bind(convert_divmod, std::placeholders::_1, false, 1)},
		{typeid(jive::bitumod_op), std::bind(convert_divmod, std::placeholders::_1, false, 0)},
		{
			typeid(jive::bitumulh_op),
			std::bind(convert_complex_bitbinary,
				std::placeholders::_1,
				&jive::i386::instr_int_mul_expand_unsigned::instance(), 0)
		},
		{
			typeid(jive::bitxor_op),
			std::bind(convert_bitbinary,
				std::placeholders::_1,
				&jive::i386::instr_int_xor::instance(),
				&jive::i386::instr_int_xor_immediate::instance())
		}
	});

	auto i0 = node->input(0), i1 = node->input(1);
	JIVE_DEBUG_ASSERT(i0->port().rescls() == i1->port().rescls());

	if (i0->port().rescls() == &gpr_regcls) {
		JIVE_DEBUG_ASSERT(map.find(typeid(op)) != map.end());
		return map[typeid(op)](node);
	}

	JIVE_ASSERT(0 && "Cannot handle resource class.");
}

static void
match_bitunary(jive::simple_node * node)
{
	JIVE_DEBUG_ASSERT(is<bitunary_op>(node));
	auto & op = node->operation();

	static std::unordered_map<std::type_index, const instruction*> map({
	  {typeid(bitneg_op), &i386::instr_int_neg::instance()}
	, {typeid(bitnot_op), &i386::instr_int_not::instance()}
	});

	auto i = node->input(0);
	if (i->port().rescls() == &gpr_regcls) {
		JIVE_DEBUG_ASSERT(map.find(typeid(op)) != map.end());
		auto result = instruction_op::create(node->region(), map[typeid(op)], {i->origin()})[0];
		return node->output(0)->divert_users(result);
	}

	JIVE_ASSERT(0 && "Cannot handle resource class.");
}

static void
match_bitcompare(jive::simple_node * node)
{
	JIVE_DEBUG_ASSERT(is<match_op>(node));
	auto compare = node->input(0)->origin()->node();
	JIVE_DEBUG_ASSERT(is<bitcompare_op>(node->input(0)->origin()->node()));
	auto & op = compare->operation();

	using namespace jive::i386;

	static
	std::unordered_map<std::type_index, std::pair<const instruction*, const instruction*>> map({
		{
			typeid(jive::biteq_op),
			{&instr_int_jump_equal::instance(), &instr_int_jump_equal::instance()}
		},
		{
			typeid(jive::bitne_op),
			{&instr_int_jump_notequal::instance(), &instr_int_jump_notequal::instance()}
		},
		{
			typeid(jive::bitslt_op),
			{&instr_int_jump_sless::instance(), &instr_int_jump_sgreater::instance()}
		},
		{
			typeid(jive::bitsle_op),
			{&instr_int_jump_slesseq::instance(), &instr_int_jump_sgreatereq::instance()}
		},
		{
			typeid(jive::bitsgt_op),
			{&instr_int_jump_sgreater::instance(), &instr_int_jump_sless::instance()}
		},
		{
			typeid(jive::bitsge_op),
			{&instr_int_jump_sgreatereq::instance(), &instr_int_jump_slesseq::instance()}
		},
		{
			typeid(jive::bitult_op),
			{&instr_int_jump_uless::instance(), &instr_int_jump_ugreater::instance()}
		},
		{
			typeid(jive::bitule_op),
			{&instr_int_jump_ulesseq::instance(), &instr_int_jump_ugreatereq::instance()}
		},
		{
			typeid(jive::bitugt_op),
			{&instr_int_jump_ugreater::instance(), &instr_int_jump_uless::instance()}
		},
		{
			typeid(jive::bituge_op),
			{&instr_int_jump_ugreatereq::instance(), &instr_int_jump_ulesseq::instance()}
		}
	});

	auto i0 = compare->input(0), i1 = compare->input(1);
	JIVE_DEBUG_ASSERT(i0->port().rescls() == i1->port().rescls());

	if (i0->port().rescls() == &gpr_regcls) {
		auto it = map.find(typeid(op));
		JIVE_DEBUG_ASSERT(it != map.end());
		return convert_bitcmp(node, it->second.first, it->second.second);
	}

	JIVE_ASSERT(0 && "Cannot handle resource class.");
}

static void
match_regvalue(jive::simple_node * node)
{
	JIVE_DEBUG_ASSERT(is_regvalue_node(node));
	auto regvalue = static_cast<const jive::regvalue_op*>(&node->operation());
	auto region = node->region();

	if (regvalue->regcls() == &gpr_regcls) {
		auto imm = regvalue_to_immediate(node);
		auto tmp = jive::immediate_op::create(region, imm);
		auto result = instruction_op::create(region, &instr_int_load_imm::instance(), {tmp})[0];
		return node->output(0)->divert_users(result);
	}

	JIVE_ASSERT(0 && "Cannot handle resource class.");
}

static void
match_bitload(jive::simple_node * node)
{
	JIVE_DEBUG_ASSERT(is<bitload_op>(node));

	if (node->output(0)->port().rescls() == &gpr_regcls)
		return convert_bitload_gpr(node);

	JIVE_ASSERT(0 && "Cannot handle resource class.");
}

static void
match_bitstore(jive::simple_node * node)
{
	JIVE_DEBUG_ASSERT(is<bitstore_op>(node));

	if (node->input(1)->port().rescls() == &gpr_regcls)
		return convert_bitstore_gpr(node);

	JIVE_ASSERT(0 && "Cannot handle resource class.");
}

static void
match_node(jive::simple_node * node)
{
	if (is<bitunary_op>(node))
		return match_bitunary(node);

	if (is<bitbinary_op>(node))
		return match_bitbinary(node);

	if (is<match_op>(node) && is<bitcompare_op>(node->input(0)->origin()->node()))
		return match_bitcompare(node);

	if (is_regvalue_node(node))
		return match_regvalue(node);

	if (is<bitload_op>(node))
		return match_bitload(node);

	if (is<bitstore_op>(node))
		return match_bitstore(node);
}

static void
match_region(jive::region * region)
{
	for (auto node : bottomup_traverser(region)) {
		if (is_simple_node(node)) {
			match_node(static_cast<simple_node*>(node));
			continue;
		}

		JIVE_DEBUG_ASSERT(is_structural_node(node));
		auto snode = static_cast<const jive::structural_node*>(node);
		for (size_t n = 0; n < snode->nsubregions(); n++)
			match_region(snode->subregion(n));
	}
}

void
match_instructions(jive::graph * graph)
{
	match_region(graph->root());
	graph->prune();

	/* verify that graph contains only instruction and immediate operations */

	std::function<bool(jive::region*)> verify = [&](jive::region * region)
	{
		for (auto node : topdown_traverser(region)) {
			if (auto snode = dynamic_cast<const structural_node*>(node)) {
				for (size_t n = 0; n < snode->nsubregions(); n++)
					verify(snode->subregion(n));
			}

			if (!is_instruction_node(node) && !is_immediate_node(node))
				return false;
		}

		return true;
	};

	JIVE_DEBUG_ASSERT(verify(graph->root()));
}

}}
