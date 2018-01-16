/*
 * Copyright 2010 2011 2012 2013 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <jive/rvsdg.h>
#include <jive/rvsdg/control.h>
#include <jive/types/bitstring.h>
#include <jive/types/bitstring/constant.h>
#include <jive/types/bitstring/value-representation.h>
#include <jive/types/function/fctlambda.h>
#include <jive/view.h>

static int types_bitstring_arithmetic_test_bitand(void)
{
	using namespace jive;

	jive::graph graph;

	auto s0 = graph.add_import(bittype(32), "s0");
	auto s1 = graph.add_import(bittype(32), "s1");

	auto c0 = create_bitconstant(graph.root(), 32, 3);
	auto c1 = create_bitconstant(graph.root(), 32, 5);

	auto and0 = create_bitand(32, s0, s1);
	auto and1 = create_bitand(32, c0, c1);

	graph.add_export(and0, "dummy");
	graph.add_export(and1, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(and0->node()->operation() == bitand_op(32));
	assert(and1->node()->operation() == int_constant_op(32, +1));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitand",
	types_bitstring_arithmetic_test_bitand)

static int types_bitstring_arithmetic_test_bitashr(void)
{
	using namespace jive;

	jive::graph graph;

	auto s0 = graph.add_import(bittype(32), "s0");
	auto s1 = graph.add_import(bittype(32), "s1");

	auto c0 = create_bitconstant(graph.root(), 32, 16);
	auto c1 = create_bitconstant(graph.root(), 32, -16);
	auto c2 = create_bitconstant(graph.root(), 32, 2);
	auto c3 = create_bitconstant(graph.root(), 32, 32);

	auto ashr0 = create_bitashr(32, s0, s1);
	auto ashr1 = create_bitashr(32, c0, c2);
	auto ashr2 = create_bitashr(32, c0, c3);
	auto ashr3 = create_bitashr(32, c1, c2);
	auto ashr4 = create_bitashr(32, c1, c3);

	graph.add_export(ashr0, "dummy");
	graph.add_export(ashr1, "dummy");
	graph.add_export(ashr2, "dummy");
	graph.add_export(ashr3, "dummy");
	graph.add_export(ashr4, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(ashr0->node()->operation() == bitashr_op(32));
	assert(ashr1->node()->operation() == int_constant_op(32, 4));
	assert(ashr2->node()->operation() == int_constant_op(32, 0));
	assert(ashr3->node()->operation() == int_constant_op(32, -4));
	assert(ashr4->node()->operation() == int_constant_op(32, -1));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitashr",
	types_bitstring_arithmetic_test_bitashr)

static int types_bitstring_arithmetic_test_bitdifference(void)
{
	using namespace jive;

	jive::graph graph;

	auto s0 = graph.add_import(bittype(32), "s0");
	auto s1 = graph.add_import(bittype(32), "s1");

	auto diff = create_bitsub(32, s0, s1);

	graph.add_export(diff, "dummy");

	graph.normalize();
	graph.prune();
	jive::view(graph.root(), stdout);

	assert(diff->node()->operation() == bitsub_op(32));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitdifference",
	types_bitstring_arithmetic_test_bitdifference)

static int types_bitstring_arithmetic_test_bitnegate(void)
{
	using namespace jive;

	jive::graph graph;

	auto s0 = graph.add_import(bittype(32), "s0");
	auto c0 = create_bitconstant(graph.root(), 32, 3);

	auto neg0 = create_bitneg(32, s0);
	auto neg1 = create_bitneg(32, c0);
	auto neg2 = create_bitneg(32, neg1);

	graph.add_export(neg0, "dummy");
	graph.add_export(neg1, "dummy");
	graph.add_export(neg2, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(neg0->node()->operation() == bitneg_op(32));
	assert(neg1->node()->operation() == int_constant_op(32, -3));
	assert(neg2->node()->operation() == int_constant_op(32, 3));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitnegate",
	types_bitstring_arithmetic_test_bitnegate)

static int types_bitstring_arithmetic_test_bitnot(void)
{
	using namespace jive;

	jive::graph graph;

	auto s0 = graph.add_import(bittype(32), "s0");
	auto c0 = create_bitconstant(graph.root(), 32, 3);

	auto not0 = create_bitnot(32, s0);
	auto not1 = create_bitnot(32, c0);
	auto not2 = create_bitnot(32, not1);

	graph.add_export(not0, "dummy");
	graph.add_export(not1, "dummy");
	graph.add_export(not2, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(not0->node()->operation() == bitnot_op(32));
	assert(not1->node()->operation() == int_constant_op(32, -4));
	assert(not2->node()->operation() == int_constant_op(32, 3));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitnot",
	types_bitstring_arithmetic_test_bitnot)

static int types_bitstring_arithmetic_test_bitor(void)
{
	using namespace jive;

	jive::graph graph;

	auto s0 = graph.add_import(bittype(32), "s0");
	auto s1 = graph.add_import(bittype(32), "s1");

	auto c0 = create_bitconstant(graph.root(), 32, 3);
	auto c1 = create_bitconstant(graph.root(), 32, 5);

	auto or0 = create_bitor(32, s0, s1);
	auto or1 = create_bitor(32, c0, c1);

	graph.add_export(or0, "dummy");
	graph.add_export(or1, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(or0->node()->operation() == bitor_op(32));
	assert(or1->node()->operation() == uint_constant_op(32, 7));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitor",
	types_bitstring_arithmetic_test_bitor)

static int types_bitstring_arithmetic_test_bitproduct(void)
{
	using namespace jive;

	jive::graph graph;

	auto s0 = graph.add_import(bittype(32), "s0");
	auto s1 = graph.add_import(bittype(32), "s1");

	auto c0 = create_bitconstant(graph.root(), 32, 3);
	auto c1 = create_bitconstant(graph.root(), 32, 5);

	auto product0 = create_bitmul(32, s0, s1);
	auto product1 = create_bitmul(32, c0, c1);

	graph.add_export(product0, "dummy");
	graph.add_export(product1, "dummy");

	graph.normalize();
	graph.prune();
	jive::view(graph.root(), stdout);

	assert(product0->node()->operation() == bitmul_op(32));
	assert(product1->node()->operation() == uint_constant_op(32, 15));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitproduct",
	types_bitstring_arithmetic_test_bitproduct)

static int types_bitstring_arithmetic_test_bitshiproduct(void)
{
	using namespace jive;

	jive::graph graph;

	auto s0 = graph.add_import(bittype(32), "s0");
	auto s1 = graph.add_import(bittype(32), "s1");

	auto shiproduct = create_bitsmulh(32, s0, s1);

	graph.add_export(shiproduct, "dummy");

	graph.normalize();
	graph.prune();
	jive::view(graph.root(), stdout);

	assert(shiproduct->node()->operation() == bitsmulh_op(32));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitshiproduct",
	types_bitstring_arithmetic_test_bitshiproduct)

static int types_bitstring_arithmetic_test_bitshl(void)
{
	using namespace jive;

	jive::graph graph;

	auto s0 = graph.add_import(bittype(32), "s0");
	auto s1 = graph.add_import(bittype(32), "s1");

	auto c0 = create_bitconstant(graph.root(), 32, 16);
	auto c1 = create_bitconstant(graph.root(), 32, 2);
	auto c2 = create_bitconstant(graph.root(), 32, 32);

	auto shl0 = create_bitshl(32, s0, s1);
	auto shl1 = create_bitshl(32, c0, c1);
	auto shl2 = create_bitshl(32, c0, c2);

	graph.add_export(shl0, "dummy");
	graph.add_export(shl1, "dummy");
	graph.add_export(shl2, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(shl0->node()->operation() == bitshl_op(32));
	assert(shl1->node()->operation() == uint_constant_op(32, 64));
	assert(shl2->node()->operation() == uint_constant_op(32, 0));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitshl",
	types_bitstring_arithmetic_test_bitshl)

static int types_bitstring_arithmetic_test_bitshr(void)
{
	using namespace jive;

	jive::graph graph;

	auto s0 = graph.add_import(bittype(32), "s0");
	auto s1 = graph.add_import(bittype(32), "s1");

	auto c0 = create_bitconstant(graph.root(), 32, 16);
	auto c1 = create_bitconstant(graph.root(), 32, 2);
	auto c2 = create_bitconstant(graph.root(), 32, 32);

	auto shr0 = create_bitshr(32, s0, s1);
	auto shr1 = create_bitshr(32, c0, c1);
	auto shr2 = create_bitshr(32, c0, c2);

	graph.add_export(shr0, "dummy");
	graph.add_export(shr1, "dummy");
	graph.add_export(shr2, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);
	
	assert(shr0->node()->operation() == bitshr_op(32));
	assert(shr1->node()->operation() == uint_constant_op(32, 4));
	assert(shr2->node()->operation() == uint_constant_op(32, 0));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitshr",
	types_bitstring_arithmetic_test_bitshr)

static int types_bitstring_arithmetic_test_bitsmod(void)
{
	using namespace jive;

	jive::graph graph;

	auto s0 = graph.add_import(bittype(32), "s0");
	auto s1 = graph.add_import(bittype(32), "s1");

	auto c0 = create_bitconstant(graph.root(), 32, -7);
	auto c1 = create_bitconstant(graph.root(), 32, 3);

	auto smod0 = create_bitsmod(32, s0, s1);
	auto smod1 = create_bitsmod(32, c0, c1);

	graph.add_export(smod0, "dummy");
	graph.add_export(smod1, "dummy");

	graph.normalize();
	graph.prune();
	jive::view(graph.root(), stdout);

	assert(smod0->node()->operation() == bitsmod_op(32));
	assert(smod1->node()->operation() == int_constant_op(32, -1));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitsmod",
	types_bitstring_arithmetic_test_bitsmod)

static int types_bitstring_arithmetic_test_bitsquotient(void)
{
	using namespace jive;

	jive::graph graph;

	auto s0 = graph.add_import(bittype(32), "s0");
	auto s1 = graph.add_import(bittype(32), "s1");

	auto c0 = create_bitconstant(graph.root(), 32, 7);
	auto c1 = create_bitconstant(graph.root(), 32, -3);

	auto squot0 = create_bitsdiv(32, s0, s1);
	auto squot1 = create_bitsdiv(32, c0, c1);

	graph.add_export(squot0, "dummy");
	graph.add_export(squot1, "dummy");

	graph.normalize();
	graph.prune();
	jive::view(graph.root(), stdout);

	assert(squot0->node()->operation() == bitsdiv_op(32));
	assert(squot1->node()->operation() == int_constant_op(32, -2));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitsquotient",
	types_bitstring_arithmetic_test_bitsquotient)

static int types_bitstring_arithmetic_test_bitsum(void)
{
	using namespace jive;

	jive::graph graph;

	auto s0 = graph.add_import(bittype(32), "s0");
	auto s1 = graph.add_import(bittype(32), "s1");

	auto c0 = create_bitconstant(graph.root(), 32, 3);
	auto c1 = create_bitconstant(graph.root(), 32, 5);

	auto sum0 = create_bitadd(32, s0, s1);
	auto sum1 = create_bitadd(32, c0, c1);

	graph.add_export(sum0, "dummy");
	graph.add_export(sum1, "dummy");

	graph.normalize();
	graph.prune();
	jive::view(graph.root(), stdout);

	assert(sum0->node()->operation() == bitadd_op(32));
	assert(sum1->node()->operation() == int_constant_op(32, 8));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitsum",
	types_bitstring_arithmetic_test_bitsum)

static int types_bitstring_arithmetic_test_bituhiproduct(void)
{
	using namespace jive;

	jive::graph graph;

	auto s0 = graph.add_import(bittype(32), "s0");
	auto s1 = graph.add_import(bittype(32), "s1");

	auto uhiproduct = create_bitumulh(32, s0, s1);

	graph.add_export(uhiproduct, "dummy");

	graph.normalize();
	graph.prune();
	jive::view(graph.root(), stdout);

	assert(uhiproduct->node()->operation() == bitumulh_op(32));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bituhiproduct",
	types_bitstring_arithmetic_test_bituhiproduct)

static int types_bitstring_arithmetic_test_bitumod(void)
{
	using namespace jive;

	jive::graph graph;

	auto s0 = graph.add_import(bittype(32), "s0");
	auto s1 = graph.add_import(bittype(32), "s1");

	auto c0 = create_bitconstant(graph.root(), 32, 7);
	auto c1 = create_bitconstant(graph.root(), 32, 3);

	auto umod0 = create_bitumod(32, s0, s1);
	auto umod1 = create_bitumod(32, c0, c1);

	graph.add_export(umod0, "dummy");
	graph.add_export(umod1, "dummy");

	graph.normalize();
	graph.prune();
	jive::view(graph.root(), stdout);

	assert(umod0->node()->operation() == bitumod_op(32));
	assert(umod1->node()->operation() == int_constant_op(32, 1));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitumod",
	types_bitstring_arithmetic_test_bitumod)

static int types_bitstring_arithmetic_test_bituquotient(void)
{
	using namespace jive;

	jive::graph graph;

	auto s0 = graph.add_import(bittype(32), "s0");
	auto s1 = graph.add_import(bittype(32), "s1");

	auto c0 = create_bitconstant(graph.root(), 32, 7);
	auto c1 = create_bitconstant(graph.root(), 32, 3);

	auto uquot0 = create_bitudiv(32, s0, s1);
	auto uquot1 = create_bitudiv(32, c0, c1);

	graph.add_export(uquot0, "dummy");
	graph.add_export(uquot1, "dummy");

	graph.normalize();
	graph.prune();
	jive::view(graph.root(), stdout);

	assert(uquot0->node()->operation() == bitudiv_op(32));
	assert(uquot1->node()->operation() == int_constant_op(32, 2));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bituquotient",
	types_bitstring_arithmetic_test_bituquotient)

static int types_bitstring_arithmetic_test_bitxor(void)
{
	using namespace jive;

	jive::graph graph;

	auto s0 = graph.add_import(bittype(32), "s0");
	auto s1 = graph.add_import(bittype(32), "s1");

	auto c0 = create_bitconstant(graph.root(), 32, 3);
	auto c1 = create_bitconstant(graph.root(), 32, 5);

	auto xor0 = create_bitxor(32, s0, s1);
	auto xor1 = create_bitxor(32, c0, c1);

	graph.add_export(xor0, "dummy");
	graph.add_export(xor1, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(xor0->node()->operation() == bitxor_op(32));
	assert(xor1->node()->operation() == int_constant_op(32, 6));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitxor",
	types_bitstring_arithmetic_test_bitxor)

static inline void
expect_static_true(jive::output * port)
{
	auto op = dynamic_cast<const jive::bitconstant_op*>(&port->node()->operation());
	assert(op && op->value().nbits() == 1 && op->value().str() == "1");
}

static inline void
expect_static_false(jive::output * port)
{
	auto op = dynamic_cast<const jive::bitconstant_op*>(&port->node()->operation());
	assert(op && op->value().nbits() == 1 && op->value().str() == "0");
}

static int types_bitstring_comparison_test_bitequal(void)
{
	using namespace jive;

	jive::graph graph;

	auto s0 = graph.add_import(bittype(32), "s0");
	auto s1 = graph.add_import(bittype(32), "s1");
	auto c0 = create_bitconstant(graph.root(), 32, 4);
	auto c1 = create_bitconstant(graph.root(), 32, 5);
	auto c2 = create_bitconstant_undefined(graph.root(), 32);

	auto equal0 = create_biteq(32, s0, s1);
	auto equal1 = create_biteq(32, c0, c0);
	auto equal2 = create_biteq(32, c0, c1);
	auto equal3 = create_biteq(32, c0, c2);

	graph.add_export(equal0, "dummy");
	graph.add_export(equal1, "dummy");
	graph.add_export(equal2, "dummy");
	graph.add_export(equal3, "dummy");
	
	graph.prune();
	jive::view(graph.root(), stdout);

	assert(equal0->node()->operation() == biteq_op(32));
	expect_static_true(equal1);
	expect_static_false(equal2);
	assert(equal3->node()->operation() == biteq_op(32));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitequal",
	types_bitstring_comparison_test_bitequal)

static int types_bitstring_comparison_test_bitnotequal(void)
{
	using namespace jive;

	jive::graph graph;

	auto s0 = graph.add_import(bittype(32), "s0");
	auto s1 = graph.add_import(bittype(32), "s1");
	auto c0 = create_bitconstant(graph.root(), 32, 4);
	auto c1 = create_bitconstant(graph.root(), 32, 5);
	auto c2 = create_bitconstant_undefined(graph.root(), 32);

	auto nequal0 = create_bitne(32, s0, s1);
	auto nequal1 = create_bitne(32, c0, c0);
	auto nequal2 = create_bitne(32, c0, c1);
	auto nequal3 = create_bitne(32, c0, c2);

	graph.add_export(nequal0, "dummy");
	graph.add_export(nequal1, "dummy");
	graph.add_export(nequal2, "dummy");
	graph.add_export(nequal3, "dummy");
	
	graph.prune();
	jive::view(graph.root(), stdout);

	assert(nequal0->node()->operation() == bitne_op(32));
	expect_static_false(nequal1);
	expect_static_true(nequal2);
	assert(nequal3->node()->operation() == bitne_op(32));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitnotequal",
	types_bitstring_comparison_test_bitnotequal)

static int types_bitstring_comparison_test_bitsgreater(void)
{
	using namespace jive;

	jive::graph graph;

	auto s0 = graph.add_import(bittype(32), "s0");
	auto s1 = graph.add_import(bittype(32), "s1");
	auto c0 = create_bitconstant(graph.root(), 32, 4);
	auto c1 = create_bitconstant(graph.root(), 32, 5);
	auto c2 = create_bitconstant(graph.root(), 32, 0x7fffffffL);
	auto c3 = create_bitconstant(graph.root(), 32, (-0x7fffffffL-1));

	auto sgreater0 = create_bitsgt(32, s0, s1);
	auto sgreater1 = create_bitsgt(32, c0, c1);
	auto sgreater2 = create_bitsgt(32, c1, c0);
	auto sgreater3 = create_bitsgt(32, s0, c2);
	auto sgreater4 = create_bitsgt(32, c3, s1);

	graph.add_export(sgreater0, "dummy");
	graph.add_export(sgreater1, "dummy");
	graph.add_export(sgreater2, "dummy");
	graph.add_export(sgreater3, "dummy");
	graph.add_export(sgreater4, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(sgreater0->node()->operation() == bitsgt_op(32));
	expect_static_false(sgreater1);
	expect_static_true(sgreater2);
	expect_static_false(sgreater3);
	expect_static_false(sgreater4);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitsgreater",
	types_bitstring_comparison_test_bitsgreater)

static int types_bitstring_comparison_test_bitsgreatereq(void)
{
	using namespace jive;

	jive::graph graph;

	auto s0 = graph.add_import(bittype(32), "s0");
	auto s1 = graph.add_import(bittype(32), "s1");
	auto c0 = create_bitconstant(graph.root(), 32, 4);
	auto c1 = create_bitconstant(graph.root(), 32, 5);
	auto c2 = create_bitconstant(graph.root(), 32, 0x7fffffffL);
	auto c3 = create_bitconstant(graph.root(), 32, (-0x7fffffffL-1));

	auto sgreatereq0 = create_bitsge(32, s0, s1);
	auto sgreatereq1 = create_bitsge(32, c0, c1);
	auto sgreatereq2 = create_bitsge(32, c1, c0);
	auto sgreatereq3 = create_bitsge(32, c0, c0);
	auto sgreatereq4 = create_bitsge(32, c2, s0);
	auto sgreatereq5 = create_bitsge(32, s1, c3);

	graph.add_export(sgreatereq0, "dummy");
	graph.add_export(sgreatereq1, "dummy");
	graph.add_export(sgreatereq2, "dummy");
	graph.add_export(sgreatereq3, "dummy");
	graph.add_export(sgreatereq4, "dummy");
	graph.add_export(sgreatereq5, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(sgreatereq0->node()->operation() == bitsge_op(32));
	expect_static_false(sgreatereq1);
	expect_static_true(sgreatereq2);
	expect_static_true(sgreatereq3);
	expect_static_true(sgreatereq4);
	expect_static_true(sgreatereq5);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitsgreatereq",
	types_bitstring_comparison_test_bitsgreatereq)

static int types_bitstring_comparison_test_bitsless(void)
{
	using namespace jive;

	jive::graph graph;

	auto s0 = graph.add_import(bittype(32), "s0");
	auto s1 = graph.add_import(bittype(32), "s1");
	auto c0 = create_bitconstant(graph.root(), 32, 4);
	auto c1 = create_bitconstant(graph.root(), 32, 5);
	auto c2 = create_bitconstant(graph.root(), 32, 0x7fffffffL);
	auto c3 = create_bitconstant(graph.root(), 32, (-0x7fffffffL-1));

	auto sless0 = create_bitslt(32, s0, s1);
	auto sless1 = create_bitslt(32, c0, c1);
	auto sless2 = create_bitslt(32, c1, c0);
	auto sless3 = create_bitslt(32, c2, s0);
	auto sless4 = create_bitslt(32, s1, c3);

	graph.add_export(sless0, "dummy");
	graph.add_export(sless1, "dummy");
	graph.add_export(sless2, "dummy");
	graph.add_export(sless3, "dummy");
	graph.add_export(sless4, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(sless0->node()->operation() == bitslt_op(32));
	expect_static_true(sless1);
	expect_static_false(sless2);
	expect_static_false(sless3);
	expect_static_false(sless4);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitsless",
	types_bitstring_comparison_test_bitsless)

static int types_bitstring_comparison_test_bitslesseq(void)
{
	using namespace jive;

	jive::graph graph;

	auto s0 = graph.add_import(bittype(32), "s0");
	auto s1 = graph.add_import(bittype(32), "s1");
	auto c0 = create_bitconstant(graph.root(), 32, 4);
	auto c1 = create_bitconstant(graph.root(), 32, 5);
	auto c2 = create_bitconstant(graph.root(), 32, 0x7fffffffL);
	auto c3 = create_bitconstant(graph.root(), 32, (-0x7fffffffL-1));

	auto slesseq0 = create_bitsle(32, s0, s1);
	auto slesseq1 = create_bitsle(32, c0, c1);
	auto slesseq2 = create_bitsle(32, c0, c0);
	auto slesseq3 = create_bitsle(32, c1, c0);
	auto slesseq4 = create_bitsle(32, s0, c2);
	auto slesseq5 = create_bitsle(32, c3, s1);

	graph.add_export(slesseq0, "dummy");
	graph.add_export(slesseq1, "dummy");
	graph.add_export(slesseq2, "dummy");
	graph.add_export(slesseq3, "dummy");
	graph.add_export(slesseq4, "dummy");
	graph.add_export(slesseq5, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(slesseq0->node()->operation() == bitsle_op(32));
	expect_static_true(slesseq1);
	expect_static_true(slesseq2);
	expect_static_false(slesseq3);
	expect_static_true(slesseq4);
	expect_static_true(slesseq5);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitslesseq",
	types_bitstring_comparison_test_bitslesseq)

static int types_bitstring_comparison_test_bitugreater(void)
{
	using namespace jive;

	jive::graph graph;

	auto s0 = graph.add_import(bittype(32), "s0");
	auto s1 = graph.add_import(bittype(32), "s1");
	auto c0 = create_bitconstant(graph.root(), 32, 4);
	auto c1 = create_bitconstant(graph.root(), 32, 5);
	auto c2 = create_bitconstant(graph.root(), 32, (0xffffffffUL));
	auto c3 = create_bitconstant(graph.root(), 32, 0);

	auto ugreater0 = create_bitugt(32, s0, s1);
	auto ugreater1 = create_bitugt(32, c0, c1);
	auto ugreater2 = create_bitugt(32, c1, c0);
	auto ugreater3 = create_bitugt(32, s0, c2);
	auto ugreater4 = create_bitugt(32, c3, s1);

	graph.add_export(ugreater0, "dummy");
	graph.add_export(ugreater1, "dummy");
	graph.add_export(ugreater2, "dummy");
	graph.add_export(ugreater3, "dummy");
	graph.add_export(ugreater4, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(ugreater0->node()->operation() == bitugt_op(32));
	expect_static_false(ugreater1);
	expect_static_true(ugreater2);
	expect_static_false(ugreater3);
	expect_static_false(ugreater4);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitugreater",
	types_bitstring_comparison_test_bitugreater)

static int types_bitstring_comparison_test_bitugreatereq(void)
{
	using namespace jive;

	jive::graph graph;

	auto s0 = graph.add_import(bittype(32), "s0");
	auto s1 = graph.add_import(bittype(32), "s1");
	auto c0 = create_bitconstant(graph.root(), 32, 4);
	auto c1 = create_bitconstant(graph.root(), 32, 5);
	auto c2 = create_bitconstant(graph.root(), 32, (0xffffffffUL));
	auto c3 = create_bitconstant(graph.root(), 32, 0);

	auto ugreatereq0 = create_bituge(32, s0, s1);
	auto ugreatereq1 = create_bituge(32, c0, c1);
	auto ugreatereq2 = create_bituge(32, c1, c0);
	auto ugreatereq3 = create_bituge(32, c0, c0);
	auto ugreatereq4 = create_bituge(32, c2, s0);
	auto ugreatereq5 = create_bituge(32, s1, c3);

	graph.add_export(ugreatereq0, "dummy");
	graph.add_export(ugreatereq1, "dummy");
	graph.add_export(ugreatereq2, "dummy");
	graph.add_export(ugreatereq3, "dummy");
	graph.add_export(ugreatereq4, "dummy");
	graph.add_export(ugreatereq5, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(ugreatereq0->node()->operation() == bituge_op(32));
	expect_static_false(ugreatereq1);
	expect_static_true(ugreatereq2);
	expect_static_true(ugreatereq3);
	expect_static_true(ugreatereq4);
	expect_static_true(ugreatereq5);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitugreatereq",
	types_bitstring_comparison_test_bitugreatereq)

static int types_bitstring_comparison_test_bituless(void)
{
	using namespace jive;

	jive::graph graph;

	auto s0 = graph.add_import(bittype(32), "s0");
	auto s1 = graph.add_import(bittype(32), "s1");
	auto c0 = create_bitconstant(graph.root(), 32, 4);
	auto c1 = create_bitconstant(graph.root(), 32, 5);
	auto c2 = create_bitconstant(graph.root(), 32, (0xffffffffUL));
	auto c3 = create_bitconstant(graph.root(), 32, 0);

	auto uless0 = create_bitult(32, s0, s1);
	auto uless1 = create_bitult(32, c0, c1);
	auto uless2 = create_bitult(32, c1, c0);
	auto uless3 = create_bitult(32, c2, s0);
	auto uless4 = create_bitult(32, s1, c3);

	graph.add_export(uless0, "dummy");
	graph.add_export(uless1, "dummy");
	graph.add_export(uless2, "dummy");
	graph.add_export(uless3, "dummy");
	graph.add_export(uless4, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(uless0->node()->operation() == bitult_op(32));
	expect_static_true(uless1);
	expect_static_false(uless2);
	expect_static_false(uless3);
	expect_static_false(uless4);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bituless",
	types_bitstring_comparison_test_bituless)

static int types_bitstring_comparison_test_bitulesseq(void)
{
	using namespace jive;

	jive::graph graph;

	auto s0 = graph.add_import(bittype(32), "s0");
	auto s1 = graph.add_import(bittype(32), "s1");
	auto c0 = create_bitconstant(graph.root(), 32, 4);
	auto c1 = create_bitconstant(graph.root(), 32, 5);
	auto c2 = create_bitconstant(graph.root(), 32, (0xffffffffUL));
	auto c3 = create_bitconstant(graph.root(), 32, 0);

	auto ulesseq0 = create_bitule(32, s0, s1);
	auto ulesseq1 = create_bitule(32, c0, c1);
	auto ulesseq2 = create_bitule(32, c0, c0);
	auto ulesseq3 = create_bitule(32, c1, c0);
	auto ulesseq4 = create_bitule(32, s0, c2);
	auto ulesseq5 = create_bitule(32, c3, s1);

	graph.add_export(ulesseq0, "dummy");
	graph.add_export(ulesseq1, "dummy");
	graph.add_export(ulesseq2, "dummy");
	graph.add_export(ulesseq3, "dummy");
	graph.add_export(ulesseq4, "dummy");
	graph.add_export(ulesseq5, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(ulesseq0->node()->operation() == bitule_op(32));
	expect_static_true(ulesseq1);
	expect_static_true(ulesseq2);
	expect_static_false(ulesseq3);
	expect_static_true(ulesseq4);
	expect_static_true(ulesseq5);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitulesseq",
	types_bitstring_comparison_test_bitulesseq)

#define ZERO_64 \
	"00000000" "00000000" "00000000" "00000000" \
	"00000000" "00000000" "00000000" "00000000"
#define ONE_64 \
	"10000000" "00000000" "00000000" "00000000" \
	"00000000" "00000000" "00000000" "00000000"
#define MONE_64 \
	"11111111" "11111111" "11111111" "11111111" \
	"11111111" "11111111" "11111111" "11111111"

static int types_bitstring_test_constant(void)
{
	using namespace jive;

	jive::graph graph;

	auto b1 = create_bitconstant(graph.root(), "00110011");
	auto b2 = create_bitconstant(graph.root(), 8, 204);
	auto b3 = create_bitconstant(graph.root(), 8, 204);
	auto b4 = create_bitconstant(graph.root(), "001100110");
	
	assert(b1->node()->operation() == uint_constant_op(8, 204));
	assert(b1->node()->operation() == int_constant_op(8, -52));

	assert(b1->node() == b2->node());
	assert(b1->node() == b3->node());
	
	assert(b1->node()->operation() == uint_constant_op(8, 204));
	assert(b1->node()->operation() == int_constant_op(8, -52));

	assert(b4->node()->operation() == uint_constant_op(9, 204));
	assert(b4->node()->operation() == int_constant_op(9, 204));

	auto plus_one_128 = create_bitconstant(graph.root(), ONE_64 ZERO_64);
	assert(plus_one_128->node()->operation() == uint_constant_op(128, 1));
	assert(plus_one_128->node()->operation() == int_constant_op(128, 1));

	auto minus_one_128 = create_bitconstant(graph.root(), MONE_64 MONE_64);
	assert(minus_one_128->node()->operation() == int_constant_op(128, -1));

	jive::view(graph.root(), stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/test-constant", types_bitstring_test_constant)

static int types_bitstring_test_normalize(void)
{
	using namespace jive;

	jive::graph graph;

	bittype bits32(32);
	auto imp = graph.add_import(bits32, "imp");

	auto c0 = create_bitconstant(graph.root(), 32, 3);
	auto c1 = create_bitconstant(graph.root(), 32, 4);
	
	auto  sum_nf = graph.node_normal_form(typeid(bitadd_op));
	assert(sum_nf);
	sum_nf->set_mutable(false);

	auto sum0 = create_bitadd(32, imp, c0);
	assert(sum0->node()->operation() == bitadd_op(32));
	assert(sum0->node()->ninputs() == 2);

	auto sum1 = create_bitadd(32, sum0, c1);
	assert(sum1->node()->operation() == bitadd_op(32));
	assert(sum1->node()->ninputs() == 2);

	auto exp = graph.add_export(sum1, "dummy");

	sum_nf->set_mutable(true);
	graph.normalize();
	graph.prune();

	assert(exp->origin()->node()->operation() == bitadd_op(32));
	assert(exp->origin()->node()->ninputs() == 2);
	auto op1 = exp->origin()->node()->input(0)->origin();
	auto op2 = exp->origin()->node()->input(1)->origin();
	if (!op1->node()) {
		auto tmp = op1; op1 = op2; op2 = tmp;
	}
	/* FIXME: the graph traversers are currently broken, that is why it won't normalize */
	assert(op1->node()->operation() == int_constant_op(32, 3 + 4));
	assert(op2 == imp);

	jive::view(graph.root(), stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/test-normalize", types_bitstring_test_normalize)

static void
assert_constant(jive::output * bitstr, size_t nbits, const char bits[])
{
	auto op = dynamic_cast<const jive::bitconstant_op &>(bitstr->node()->operation());
	assert(op.value() == jive::bitvalue_repr(std::string(bits, nbits).c_str()));
}

static int types_bitstring_test_reduction(void)
{
	using namespace jive;

	jive::graph graph;

	auto a = create_bitconstant(graph.root(), "1100");
	auto b = create_bitconstant(graph.root(), "1010");

	assert_constant(create_bitand(4, a, b), 4, "1000");
	assert_constant(create_bitor(4, a, b), 4, "1110");
	assert_constant(create_bitxor(4, a, b), 4, "0110");
	assert_constant(create_bitadd(4, a, b), 4, "0001");
	assert_constant(create_bitmul(4, a, b), 4, "1111");
	assert_constant(jive_bitconcat({a, b}), 8, "11001010");
	assert_constant(create_bitneg(4, a), 4, "1011");
	assert_constant(create_bitneg(4, b), 4, "1101");
	
	graph.prune();
	
	auto x = graph.add_import(bittype(16), "x");
	auto y = graph.add_import(bittype(16), "y");
	
	{
		auto concat = jive_bitconcat({x, y});
		auto slice = jive_bitslice(concat, 8, 24);
		auto node = slice->node();
		assert(dynamic_cast<const bitconcat_op*>(&node->operation()));
		assert(node->ninputs() == 2);
		assert(dynamic_cast<const bitslice_op*>(&node->input(0)->origin()->node()->operation()));
		assert(dynamic_cast<const bitslice_op*>(&node->input(1)->origin()->node()->operation()));
		
		const bitslice_op * attrs;
		attrs = dynamic_cast<const bitslice_op*>(&node->input(0)->origin()->node()->operation());
		assert( (attrs->low() == 8) && (attrs->high() == 16) );
		attrs = dynamic_cast<const bitslice_op*>(&node->input(1)->origin()->node()->operation());
		assert( (attrs->low() == 0) && (attrs->high() == 8) );
		
		assert(node->input(0)->origin()->node()->input(0)->origin() == x);
		assert(node->input(1)->origin()->node()->input(0)->origin() == y);
	}
	
	{
		auto slice1 = jive_bitslice(x, 0, 8);
		auto slice2 = jive_bitslice(x, 8, 16);
		auto concat = jive_bitconcat({slice1, slice2});
		assert(concat == x);
	}

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/test-reduction", types_bitstring_test_reduction)

static int types_bitstring_test_slice_concat(void)
{
	using namespace jive;

	jive::graph graph;
	
	auto base_const1 = create_bitconstant(graph.root(), "00110111");
	auto base_const2 = create_bitconstant(graph.root(), "11001000");
	
	auto base_x = graph.add_import(bittype(8), "x");
	auto base_y = graph.add_import(bittype(8), "y");
	auto base_z = graph.add_import(bittype(8), "z");
	
	{
		/* slice of constant */
		auto a = jive_bitslice(base_const1, 2, 6);
		
		auto & op = dynamic_cast<const bitconstant_op&>(a->node()->operation());
		assert(op.value() == bitvalue_repr("1101"));
	}
	
	{
		/* slice of slice */
		auto a = jive_bitslice(base_x, 2, 6);
		auto b = jive_bitslice(a, 1, 3);

		assert(dynamic_cast<const bitslice_op*>(&b->node()->operation()));
		const bitslice_op * attrs;
		attrs = dynamic_cast<const bitslice_op*>(&b->node()->operation());
		assert(attrs->low() == 3 && attrs->high() == 5);
	}
	
	{
		/* slice of full node */
		auto a = jive_bitslice(base_x, 0, 8);
		
		assert(a == base_x);
	}
	
	{
		/* slice of concat */
		auto a = jive_bitconcat({base_x, base_y});
		auto b = jive_bitslice(a, 0, 8);
		
		assert(static_cast<const bittype*>(&b->type())->nbits() == 8);
		
		assert(b == base_x);
	}
	
	{
		/* concat flattening */
		auto a = jive_bitconcat({base_x, base_y});
		auto b = jive_bitconcat({a, base_z});
		
		assert(dynamic_cast<const bitconcat_op*>(&b->node()->operation()));
		assert(b->node()->ninputs() == 3);
		assert(b->node()->input(0)->origin() == base_x);
		assert(b->node()->input(1)->origin() == base_y);
		assert(b->node()->input(2)->origin() == base_z);
	}
	
	{
		/* concat of single node */
		auto a = jive_bitconcat({base_x});
		
		assert(a==base_x);
	}
	
	{
		/* concat of slices */
		auto a = jive_bitslice(base_x, 0, 4);
		auto b = jive_bitslice(base_x, 4, 8);
		auto c = jive_bitconcat({a, b});
		
		assert(c==base_x);
	}
	
	{
		/* concat of constants */
		auto a = jive_bitconcat({base_const1, base_const2});
		
		auto & op = dynamic_cast<const bitconstant_op&>(a->node()->operation());
		assert(op.value() == bitvalue_repr("0011011111001000"));
	}
	
	{
		/* CSE */
		auto b = create_bitconstant(graph.root(), "00110111");
		assert(b == base_const1);
		
		auto c = jive_bitslice(base_x, 2, 6);
		auto d = jive_bitslice(base_x, 2, 6);
		assert(c == d);
		
		auto e = jive_bitconcat({base_x, base_y});
		auto f = jive_bitconcat({base_x, base_y});
		assert(e == f);
	}
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/test-slice-concat", types_bitstring_test_slice_concat)

static const char * bs[] = {
	"00000000",
	"11111111",
	"10000000",
	"01111111",
	"00001111",
	"XXXX0011",
	"XD001100",
	"XXXXDDDD",
	"10XDDX01",
	"0DDDDDD1"};

static std::string bitstring_not[] = {
	"11111111",
	"00000000",
	"01111111",
	"10000000",
	"11110000",
	"XXXX1100",
	"XD110011",
	"XXXXDDDD",
	"01XDDX10",
	"1DDDDDD0"};

static std::string bitstring_xor[10][10] = {
	{"00000000", "11111111", "10000000", "01111111", "00001111", "XXXX0011", "XD001100", "XXXXDDDD",
		"10XDDX01", "0DDDDDD1"},
	{"11111111", "00000000", "01111111", "10000000", "11110000", "XXXX1100", "XD110011", "XXXXDDDD",
		"01XDDX10", "1DDDDDD0"},
	{"10000000", "01111111", "00000000", "11111111", "10001111", "XXXX0011", "XD001100", "XXXXDDDD",
		"00XDDX01", "1DDDDDD1"},
	{"01111111", "10000000", "11111111", "00000000", "01110000", "XXXX1100", "XD110011", "XXXXDDDD",
		"11XDDX10", "0DDDDDD0"},
	{"00001111", "11110000", "10001111", "01110000", "00000000", "XXXX1100", "XD000011", "XXXXDDDD",
		"10XDDX10", "0DDDDDD0"},
	{"XXXX0011", "XXXX1100", "XXXX0011", "XXXX1100", "XXXX1100", "XXXX0000", "XXXX1111", "XXXXDDDD",
		"XXXXDX10", "XXXXDDD0"},
	{"XD001100", "XD110011", "XD001100", "XD110011", "XD000011", "XXXX1111", "XD000000", "XXXXDDDD",
		"XDXDDX01", "XDDDDDD1"},
	{"XXXXDDDD", "XXXXDDDD", "XXXXDDDD", "XXXXDDDD", "XXXXDDDD", "XXXXDDDD", "XXXXDDDD", "XXXXDDDD",
		"XXXXDXDD", "XXXXDDDD"},
	{"10XDDX01", "01XDDX10", "00XDDX01", "11XDDX10", "10XDDX10", "XXXXDX10", "XDXDDX01", "XXXXDXDD",
		"00XDDX00", "1DXDDXD0"},
	{"0DDDDDD1", "1DDDDDD0", "1DDDDDD1", "0DDDDDD0", "0DDDDDD0", "XXXXDDD0", "XDDDDDD1", "XXXXDDDD",
		"1DXDDXD0", "0DDDDDD0"}};

static std::string bitstring_or[10][10] = {
	{"00000000", "11111111", "10000000", "01111111", "00001111", "XXXX0011", "XD001100", "XXXXDDDD",
		"10XDDX01", "0DDDDDD1"},
	{"11111111", "11111111", "11111111", "11111111", "11111111", "11111111", "11111111", "11111111",
		"11111111", "11111111"},
	{"10000000", "11111111", "10000000", "11111111", "10001111", "1XXX0011", "1D001100", "1XXXDDDD",
		"10XDDX01", "1DDDDDD1"},
	{"01111111", "11111111", "11111111", "01111111", "01111111", "X1111111", "X1111111", "X1111111",
		"11111111", "01111111"},
	{"00001111", "11111111", "10001111", "01111111", "00001111", "XXXX1111", "XD001111", "XXXX1111",
		"10XD1111", "0DDD1111"},
	{"XXXX0011", "11111111", "1XXX0011", "X1111111", "XXXX1111", "XXXX0011", "XXXX1111", "XXXXDD11",
		"1XXXDX11", "XXXXDD11"},
	{"XD001100", "11111111", "1D001100", "X1111111", "XD001111", "XXXX1111", "XD001100", "XXXX11DD",
		"1DXD1101", "XDDD11D1"},
	{"XXXXDDDD", "11111111", "1XXXDDDD", "X1111111", "XXXX1111", "XXXXDD11", "XXXX11DD", "XXXXDDDD",
		"1XXXDXD1", "XXXXDDD1"},
	{"10XDDX01", "11111111", "10XDDX01", "11111111", "10XD1111", "1XXXDX11", "1DXD1101", "1XXXDXD1",
		"10XDDX01", "1DXDDXD1"},
	{"0DDDDDD1", "11111111", "1DDDDDD1", "01111111", "0DDD1111", "XXXXDD11", "XDDD11D1", "XXXXDDD1",
		"1DXDDXD1", "0DDDDDD1"}};

static std::string bitstring_and[10][10] = {
	{"00000000", "00000000", "00000000", "00000000", "00000000", "00000000", "00000000", "00000000",
		"00000000", "00000000"},
	{"00000000", "11111111", "10000000", "01111111", "00001111", "XXXX0011", "XD001100", "XXXXDDDD",
		"10XDDX01", "0DDDDDD1"},
	{"00000000", "10000000", "10000000", "00000000", "00000000", "X0000000", "X0000000", "X0000000",
		"10000000", "00000000"},
	{"00000000", "01111111", "00000000", "01111111", "00001111", "0XXX0011", "0D001100", "0XXXDDDD",
		"00XDDX01", "0DDDDDD1"},
	{"00000000", "00001111", "00000000", "00001111", "00001111", "00000011", "00001100", "0000DDDD",
		"0000DX01", "0000DDD1"},
	{"00000000", "XXXX0011", "X0000000", "0XXX0011", "00000011", "XXXX0011", "XX000000", "XXXX00DD",
		"X0XX0001", "0XXX00D1"},
	{"00000000", "XD001100", "X0000000", "0D001100", "00001100", "XX000000", "XD001100", "XX00DD00",
		"X000DX00", "0D00DD00"},
	{"00000000", "XXXXDDDD", "X0000000", "0XXXDDDD", "0000DDDD", "XXXX00DD", "XX00DD00", "XXXXDDDD",
		"X0XXDX0D", "0XXXDDDD"},
	{"00000000", "10XDDX01", "10000000", "00XDDX01", "0000DX01", "X0XX0001", "X000DX00", "X0XXDX0D",
		"10XDDX01", "00XDDX01"},
	{"00000000", "0DDDDDD1", "00000000", "0DDDDDD1", "0000DDD1", "0XXX00D1", "0D00DD00", "0XXXDDDD",
		"00XDDX01", "0DDDDDD1"}};

static char equal[10][10] = {
	{'1', '0', '0', '0', '0', '0', '0', 'X', '0', '0'},
	{'0', '1', '0', '0', '0', '0', '0', 'X', '0', '0'},
	{'0', '0', '1', '0', '0', '0', '0', 'X', '0', '0'},
	{'0', '0', '0', '1', '0', '0', '0', 'X', '0', 'D'},
	{'0', '0', '0', '0', '1', '0', '0', 'X', '0', 'D'},
	{'0', '0', '0', '0', '0', 'X', '0', 'X', '0', 'X'},
	{'0', '0', '0', '0', '0', '0', 'X', 'X', '0', '0'},
	{'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X'},
	{'0', '0', '0', '0', '0', '0', '0', 'X', 'X', '0'},
	{'0', '0', '0', 'D', 'D', 'X', '0', 'X', '0', 'D'}};

static char notequal[10][10] = {
	{'0', '1', '1', '1', '1', '1', '1', 'X', '1', '1'},
	{'1', '0', '1', '1', '1', '1', '1', 'X', '1', '1'},
	{'1', '1', '0', '1', '1', '1', '1', 'X', '1', '1'},
	{'1', '1', '1', '0', '1', '1', '1', 'X', '1', 'D'},
	{'1', '1', '1', '1', '0', '1', '1', 'X', '1', 'D'},
	{'1', '1', '1', '1', '1', 'X', '1', 'X', '1', 'X'},
	{'1', '1', '1', '1', '1', '1', 'X', 'X', '1', '1'},
	{'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X'},
	{'1', '1', '1', '1', '1', '1', '1', 'X', 'X', '1'},
	{'1', '1', '1', 'D', 'D', 'X', '1', 'X', '1', 'D'}};

static char sgreatereq[10][10] = {
	{'1', '1', '0', '1', '1', '1', '0', 'X', '1', '1'},
	{'0', '1', '0', '1', '1', '1', '0', 'D', '1', '1'},
	{'1', '1', '1', '1', '1', '1', '0', 'X', '1', '1'},
	{'0', '0', '0', '1', '1', '1', '0', 'X', '1', '1'},
	{'0', '0', '0', '0', '1', '1', '0', 'X', '1', 'D'},
	{'0', '0', '0', '0', '0', 'X', '0', 'X', '1', 'X'},
	{'1', '1', '1', '1', '1', '1', 'X', 'X', '1', '1'},
	{'D', 'X', 'X', 'X', 'D', 'X', 'X', 'X', 'X', 'X'},
	{'0', '0', '0', '0', '0', '0', '0', 'X', 'X', 'X'},
	{'0', '0', '0', 'D', 'D', 'X', '0', 'X', 'X', 'D'}};

static char sgreater[10][10] = {
	{'0', '1', '0', '1', '1', '1', '0', 'D', '1', '1'},
	{'0', '0', '0', '1', '1', '1', '0', 'X', '1', '1'},
	{'1', '1', '0', '1', '1', '1', '0', 'X', '1', '1'},
	{'0', '0', '0', '0', '1', '1', '0', 'X', '1', 'D'},
	{'0', '0', '0', '0', '0', '1', '0', 'D', '1', 'D'},
	{'0', '0', '0', '0', '0', 'X', '0', 'X', '1', 'X'},
	{'1', '1', '1', '1', '1', '1', 'X', 'X', '1', '1'},
	{'X', 'D', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X'},
	{'0', '0', '0', '0', '0', '0', '0', 'X', 'X', 'X'},
	{'0', '0', '0', '0', 'D', 'X', '0', 'X', 'X', 'D'}};

static char slesseq[10][10] = {
	{'1', '0', '1', '0', '0', '0', '1', 'D', '0', '0'},
	{'1', '1', '1', '0', '0', '0', '1', 'X', '0', '0'},
	{'0', '0', '1', '0', '0', '0', '1', 'X', '0', '0'},
	{'1', '1', '1', '1', '0', '0', '1', 'X', '0', 'D'},
	{'1', '1', '1', '1', '1', '0', '1', 'D', '0', 'D'},
	{'1', '1', '1', '1', '1', 'X', '1', 'X', '0', 'X'},
	{'0', '0', '0', '0', '0', '0', 'X', 'X', '0', '0'},
	{'X', 'D', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X'},
	{'1', '1', '1', '1', '1', '1', '1', 'X', 'X', 'X'},
	{'1', '1', '1', '1', 'D', 'X', '1', 'X', 'X', 'D'}};

static char sless[10][10] = {
	{'0', '0', '1', '0', '0', '0', '1', 'X', '0', '0'},
	{'1', '0', '1', '0', '0', '0', '1', 'D', '0', '0'},
	{'0', '0', '0', '0', '0', '0', '1', 'X', '0', '0'},
	{'1', '1', '1', '0', '0', '0', '1', 'X', '0', '0'},
	{'1', '1', '1', '1', '0', '0', '1', 'X', '0', 'D'},
	{'1', '1', '1', '1', '1', 'X', '1', 'X', '0', 'X'},
	{'0', '0', '0', '0', '0', '0', 'X', 'X', '0', '0'},
	{'D', 'X', 'X', 'X', 'D', 'X', 'X', 'X', 'X', 'X'},
	{'1', '1', '1', '1', '1', '1', '1', 'X', 'X', 'X'},
	{'1', '1', '1', 'D', 'D', 'X', '1', 'X', 'X', 'D'}};

static char ugreatereq[10][10] = {
	{'1', '0', '0', '0', '0', '0', '0', 'X', '0', '0'},
	{'1', '1', '1', '1', '1', '1', '1', '1', '1', '1'},
	{'1', '0', '1', '0', '0', '0', '0', 'X', '0', '0'},
	{'1', '0', '1', '1', '1', '1', '1', 'X', '1', '1'},
	{'1', '0', '1', '0', '1', '1', '1', 'X', '1', 'D'},
	{'1', '0', '1', '0', '0', 'X', '1', 'X', '1', 'X'},
	{'1', '0', '1', '0', '0', '0', 'X', 'X', '0', '0'},
	{'1', 'X', 'X', 'X', 'D', 'X', 'X', 'X', 'X', 'X'},
	{'1', '0', '1', '0', '0', '0', '1', 'X', 'X', 'X'},
	{'1', '0', '1', 'D', 'D', 'X', '1', 'X', 'X', 'D'}};

static char ugreater[10][10] = {
	{'0', '0', '0', '0', '0', '0', '0', '0', '0', '0'},
	{'1', '0', '1', '1', '1', '1', '1', 'X', '1', '1'},
	{'1', '0', '0', '0', '0', '0', '0', 'X', '0', '0'},
	{'1', '0', '1', '0', '1', '1', '1', 'X', '1', 'D'},
	{'1', '0', '1', '0', '0', '1', '1', 'D', '1', 'D'},
	{'1', '0', '1', '0', '0', 'X', '1', 'X', '1', 'X'},
	{'1', '0', '1', '0', '0', '0', 'X', 'X', '0', '0'},
	{'X', '0', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X'},
	{'1', '0', '1', '0', '0', '0', '1', 'X', 'X', 'X'},
	{'1', '0', '1', '0', 'D', 'X', '1', 'X', 'X', 'D'}};

static char ulesseq[10][10] = {
	{'1', '1', '1', '1', '1', '1', '1', '1', '1', '1'},
	{'0', '1', '0', '0', '0', '0', '0', 'X', '0', '0'},
	{'0', '1', '1', '1', '1', '1', '1', 'X', '1', '1'},
	{'0', '1', '0', '1', '0', '0', '0', 'X', '0', 'D'},
	{'0', '1', '0', '1', '1', '0', '0', 'D', '0', 'D'},
	{'0', '1', '0', '1', '1', 'X', '0', 'X', '0', 'X'},
	{'0', '1', '0', '1', '1', '1', 'X', 'X', '1', '1'},
	{'X', '1', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X'},
	{'0', '1', '0', '1', '1', '1', '0', 'X', 'X', 'X'},
	{'0', '1', '0', '1', 'D', 'X', '0', 'X', 'X', 'D'}};

static char uless[10][10] = {
	{'0', '1', '1', '1', '1', '1', '1', 'X', '1', '1'},
	{'0', '0', '0', '0', '0', '0', '0', '0', '0', '0'},
	{'0', '1', '0', '1', '1', '1', '1', 'X', '1', '1'},
	{'0', '1', '0', '0', '0', '0', '0', 'X', '0', '0'},
	{'0', '1', '0', '1', '0', '0', '0', 'X', '0', 'D'},
	{'0', '1', '0', '1', '1', 'X', '0', 'X', '0', 'X'},
	{'0', '1', '0', '1', '1', '1', 'X', 'X', '1', '1'},
	{'0', 'X', 'X', 'X', 'D', 'X', 'X', 'X', 'X', 'X'},
	{'0', '1', '0', '1', '1', '1', '0', 'X', 'X', 'X'},
	{'0', '1', '0', 'D', 'D', 'X', '0', 'X', 'X', 'D'}};

static int
types_bitstring_test_value_representation()
{
	using namespace jive;

	for (size_t r = 0; r < 10; r++) {
		assert(bitvalue_repr(bs[r]).lnot() == bitstring_not[r]);
		for (size_t c = 0; c < 10; c++) {
			assert(bitvalue_repr(bs[r]).land(bs[c]) == bitstring_and[r][c]);
			assert(bitvalue_repr(bs[r]).lor(bs[c]) == bitstring_or[r][c]);
			assert(bitvalue_repr(bs[r]).lxor(bs[c]) == bitstring_xor[r][c]);

			assert(bitvalue_repr(bs[r]).ult(bs[c]) == uless[r][c]);
			assert(bitvalue_repr(bs[r]).slt(bs[c]) == sless[r][c]);

			assert(bitvalue_repr(bs[r]).ule(bs[c]) == ulesseq[r][c]);
			assert(bitvalue_repr(bs[r]).sle(bs[c]) == slesseq[r][c]);

			assert(bitvalue_repr(bs[r]).eq(bs[c]) == equal[r][c]);
			assert(bitvalue_repr(bs[r]).ne(bs[c]) == notequal[r][c]);

			assert(bitvalue_repr(bs[r]).uge(bs[c]) == ugreatereq[r][c]);
			assert(bitvalue_repr(bs[r]).sge(bs[c]) == sgreatereq[r][c]);

			assert(bitvalue_repr(bs[r]).ugt(bs[c]) == ugreater[r][c]);
			assert(bitvalue_repr(bs[r]).sgt(bs[c]) == sgreater[r][c]);
		}
	}

	assert(bitvalue_repr("000110").to_uint() == 24);
	assert(bitvalue_repr("00011").to_int() == -8);

	for(ssize_t r = -4; r < 5; r++){
		bitvalue_repr rbits(32, r);

		assert(rbits.neg() == -r);
		assert(rbits.shl(1) == r << 1);
		assert(rbits.shl(32) == 0);
		assert(rbits.ashr(1) == r >> 1);
		assert(rbits.ashr(34) == (r < 0 ? -1 : 0));

		if (r >= 0) {
			assert(rbits.shr(1) == r >> 1);
			assert(rbits.shr(34) == 0);
		}

		for (ssize_t c = -4; c < 5; c++) {
			bitvalue_repr cbits(32, c);

			assert(rbits.add(cbits) == r+c);
			assert(rbits.sub(cbits) == r-c);
			assert(rbits.mul(cbits) == r*c);

			if (r >= 0 && c > 0) {
				assert(rbits.udiv(cbits) == r/c);
				assert(rbits.umod(cbits) == r%c);
			}

			if (c != 0) {
				assert(rbits.sdiv(cbits) == r/c);
				assert(rbits.smod(cbits) == r%c);
			}
		}
	}

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/test-value-representation",
	types_bitstring_test_value_representation)
