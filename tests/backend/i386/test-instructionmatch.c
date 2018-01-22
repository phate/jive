/*
 * Copyright 2018 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <jive/arch/addresstype.h>
#include <jive/arch/load.h>
#include <jive/arch/regvalue.h>
#include <jive/arch/store.h>
#include <jive/backend/i386/instructionmatch.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/registerset.h>
#include <jive/rvsdg/control.h>
#include <jive/rvsdg/graph.h>
#include <jive/types/bitstring.h>

template<class OPERATOR> static void
setup_bitbinary(
	jive::graph & graph,
	const jive::register_class & regcls)
{
	auto i0 = graph.add_import(regcls.type(), "");
	auto i1 = graph.add_import(regcls.type(), "");

	OPERATOR op(32);
	auto node = jive::simple_node::create(graph.root(), op, {i0, i1});
	node->input(0)->replace(&regcls);
	node->input(1)->replace(&regcls);
	node->output(0)->replace(&regcls);

	graph.add_export(node->output(0), "");
}

template<class OPERATOR> static void
setup_bitunary(
	jive::graph & graph,
	const jive::register_class & regcls)
{
	auto i = graph.add_import(regcls.type(), "");

	OPERATOR op(32);
	auto node = jive::simple_node::create(graph.root(), op, {i});
	node->input(0)->replace(&regcls);
	node->output(0)->replace(&regcls);

	graph.add_export(node->output(0), "");
}

template<class OPERATOR> static void
setup_bitcompare(
	jive::graph & graph,
	const jive::register_class & iregcls,
	const jive::register_class & oregcls)
{
	auto i0 = graph.add_import(iregcls.type(), "");
	auto i1 = graph.add_import(iregcls.type(), "");

	OPERATOR op(32);
	auto cmp = jive::simple_node::create(graph.root(), op, {i0, i1});
	auto result = jive::match(1, {{0,0}}, 1, 2, cmp->output(0));

	cmp->input(0)->replace(&iregcls);
	cmp->input(1)->replace(&iregcls);
	/* FIXME: I would like to assign cc register class to its output, but cc register class has
	          type bit16 and the compare instruction expect type bit32
	          We have to convert the output of bitcompare operations to control type again in order
	          to fix this.
	*/

	graph.add_export(result, "");
}

static void
test_bitoperations()
{
	using namespace jive;

	static std::unordered_map<
		std::type_index,
		std::pair<std::function<void(graph&, const register_class & regls)>, const instruction*>
	> arithmetic_map({
	  {typeid(bitadd_op),   {setup_bitbinary<bitadd_op>, &i386::instr_int_add::instance()}}
	, {typeid(bitand_op),   {setup_bitbinary<bitand_op>, &i386::instr_int_and::instance()}}
	, {typeid(bitashr_op),  {setup_bitbinary<bitashr_op>, &i386::instr_int_ashr::instance()}}
	, {typeid(bitmul_op),   {setup_bitbinary<bitmul_op>, &i386::instr_int_mul::instance()}}
	, {typeid(bitor_op),    {setup_bitbinary<bitor_op>, &i386::instr_int_or::instance()}}
	, {typeid(bitsdiv_op),  {setup_bitbinary<bitsdiv_op>, &i386::instr_int_sdiv::instance()}}
	, {typeid(bitshl_op),   {setup_bitbinary<bitshl_op>, &i386::instr_int_shl::instance()}}
	, {typeid(bitshr_op),   {setup_bitbinary<bitshr_op>, &i386::instr_int_shr::instance()}}
	, {typeid(bitsmod_op),  {setup_bitbinary<bitsmod_op>, &i386::instr_int_sdiv::instance()}}
	, {typeid(bitsmulh_op), {setup_bitbinary<bitsmulh_op>, &i386::instr_int_mul_expand_signed::instance()}}
	, {typeid(bitudiv_op),  {setup_bitbinary<bitudiv_op>, &i386::instr_int_udiv::instance()}}
	, {typeid(bitumod_op),  {setup_bitbinary<bitumod_op>, &i386::instr_int_udiv::instance()}}
	, {typeid(bitumulh_op), {setup_bitbinary<bitumulh_op>, &i386::instr_int_mul_expand_unsigned::instance()}}
	, {typeid(bitxor_op),   {setup_bitbinary<bitxor_op>, &i386::instr_int_xor::instance()}}

	, {typeid(bitneg_op),   {setup_bitunary<bitneg_op>, &i386::instr_int_neg::instance()}}
	, {typeid(bitnot_op),   {setup_bitunary<bitnot_op>, &i386::instr_int_not::instance()}}

	});

	for (const auto & pair : arithmetic_map) {
		jive::graph graph;
		pair.second.first(graph, i386::gpr_regcls);
		i386::match_instructions(&graph);

		auto node = graph.root()->result(0)->origin()->node();
		assert(is_instruction_node(node));

		auto i = static_cast<const instruction_op*>(&node->operation())->icls();
		assert(i == pair.second.second);
	}

	static std::unordered_map<
		std::type_index,
		std::pair<
			std::function<void(graph&, const register_class&, const register_class &)>,
			const instruction*
		>
	> compare_map({
	 {typeid(biteq_op), {setup_bitcompare<biteq_op>, &i386::instr_int_jump_equal::instance()}}
	,{typeid(bitne_op), {setup_bitcompare<bitne_op>, &i386::instr_int_jump_notequal::instance()}}

	,{typeid(bitslt_op), {setup_bitcompare<bitslt_op>, &i386::instr_int_jump_sless::instance()}}
	,{typeid(bitsle_op), {setup_bitcompare<bitsle_op>, &i386::instr_int_jump_slesseq::instance()}}

	,{typeid(bitsgt_op), {setup_bitcompare<bitsgt_op>, &i386::instr_int_jump_sgreater::instance()}}
	,{typeid(bitsge_op), {setup_bitcompare<bitsge_op>, &i386::instr_int_jump_sgreatereq::instance()}}

	,{typeid(bitult_op), {setup_bitcompare<bitult_op>, &i386::instr_int_jump_uless::instance()}}
	,{typeid(bitule_op), {setup_bitcompare<bitule_op>, &i386::instr_int_jump_ulesseq::instance()}}

	,{typeid(bitugt_op), {setup_bitcompare<bitugt_op>, &i386::instr_int_jump_ugreater::instance()}}
	,{typeid(bituge_op), {setup_bitcompare<bituge_op>, &i386::instr_int_jump_ugreatereq::instance()}}
	});

	for (const auto & pair : compare_map) {
		jive::graph graph;
		pair.second.first(graph, i386::gpr_regcls, i386::cc_regcls);
		i386::match_instructions(&graph);

		auto node = graph.root()->result(0)->origin()->node();
		assert(is_instruction_node(node));

		auto i = static_cast<const instruction_op*>(&node->operation())->icls();
		assert(i == pair.second.second);
	}
}

static void
test_regvalue()
{
	using namespace jive;

	jive::graph graph;

	auto rv = regvalue_op::create(graph.root(), uint_constant_op(32, 4), &i386::gpr_regcls);

	auto x0 = graph.add_export(rv, "");

	i386::match_instructions(&graph);

	auto node = x0->origin()->node();
	assert(is_instruction_node(node));

	auto i = static_cast<const instruction_op*>(&node->operation())->icls();
	assert(i == &i386::instr_int_load_imm::instance());
}

static void
test_load()
{
	using namespace jive;

	jive::graph graph;
	auto i0 = graph.add_import(bit32, "");
	auto i1 = graph.add_import(memtype::instance(), "");

	auto l = bitload_op::create(i0, 32, bit32, {i1});
	l->node()->input(0)->replace(&i386::gpr_regcls);
	l->node()->output(0)->replace(&i386::gpr_regcls);

	auto x0 = graph.add_export(l, "");

	i386::match_instructions(&graph);

	auto node = x0->origin()->node();
	assert(is_instruction_node(node));

	auto i = static_cast<const instruction_op*>(&node->operation())->icls();
	assert(i == &i386::instr_int_load32_disp::instance());
}

static void
test_store()
{
	using namespace jive;

	jive::graph graph;
	auto i0 = graph.add_import(bit32, "");
	auto i1 = graph.add_import(bit32, "");
	auto i2 = graph.add_import(memtype::instance(), "");

	auto s = bitstore_op::create(i0, i1, 32, bit32, {i2})[0];
	s->node()->input(0)->replace(&i386::gpr_regcls);
	s->node()->input(1)->replace(&i386::gpr_regcls);

	auto x0 = graph.add_export(s, "");

	i386::match_instructions(&graph);

	auto node = x0->origin()->node();
	assert(is_instruction_node(node));

	auto i = static_cast<const instruction_op*>(&node->operation())->icls();
	assert(i == &i386::instr_int_store32_disp::instance());
}

static int
test_main()
{
	test_bitoperations();
	test_regvalue();
	test_load();
	test_store();

	return 0;
}

JIVE_UNIT_TEST_REGISTER("backend/i386/test-instructionmatch", test_main)
