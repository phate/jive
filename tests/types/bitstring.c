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

#include <jive/types/bitstring.h>
#include <jive/types/bitstring/constant.h>
#include <jive/types/bitstring/value-representation.h>
#include <jive/types/function/fctlambda.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/structural_node.h>

static int types_bitstring_arithmetic_test_bitand(void)
{
	jive_graph graph;

	auto s0 = graph.import(jive::bits::type(32), "s0");
	auto s1 = graph.import(jive::bits::type(32), "s1");

	auto c0 = jive_bitconstant_unsigned(graph.root(), 32, 3);
	auto c1 = jive_bitconstant_unsigned(graph.root(), 32, 5);

	auto and0 = dynamic_cast<jive::output*>(jive_bitand({s0, s1}));
	auto and1 = dynamic_cast<jive::output*>(jive_bitand({c0, c1}));

	graph.export_port(and0, "dummy");
	graph.export_port(and1, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(and0->node()->operation() == jive::bits::and_op(32));
	assert(and1->node()->operation() == jive::bits::int_constant_op(32, +1));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitand", types_bitstring_arithmetic_test_bitand);

static int types_bitstring_arithmetic_test_bitashr(void)
{
	jive_graph graph;

	auto s0 = graph.import(jive::bits::type(32), "s0");
	auto s1 = graph.import(jive::bits::type(32), "s1");

	auto c0 = jive_bitconstant_unsigned(graph.root(), 32, 16);
	auto c1 = jive_bitconstant_signed(graph.root(), 32, -16);
	auto c2 = jive_bitconstant_unsigned(graph.root(), 32, 2);
	auto c3 = jive_bitconstant_unsigned(graph.root(), 32, 32);

	auto ashr0 = dynamic_cast<jive::output*>(jive_bitashr(s0, s1));
	auto ashr1 = dynamic_cast<jive::output*>(jive_bitashr(c0, c2));
	auto ashr2 = dynamic_cast<jive::output*>(jive_bitashr(c0, c3));
	auto ashr3 = dynamic_cast<jive::output*>(jive_bitashr(c1, c2));
	auto ashr4 = dynamic_cast<jive::output*>(jive_bitashr(c1, c3));

	graph.export_port(ashr0, "dummy");
	graph.export_port(ashr1, "dummy");
	graph.export_port(ashr2, "dummy");
	graph.export_port(ashr3, "dummy");
	graph.export_port(ashr4, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(ashr0->node()->operation() == jive::bits::ashr_op(32));
	assert(ashr1->node()->operation() == jive::bits::int_constant_op(32, 4));
	assert(ashr2->node()->operation() == jive::bits::int_constant_op(32, 0));
	assert(ashr3->node()->operation() == jive::bits::int_constant_op(32, -4));
	assert(ashr4->node()->operation() == jive::bits::int_constant_op(32, -1));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitashr", types_bitstring_arithmetic_test_bitashr);

static int types_bitstring_arithmetic_test_bitdifference(void)
{
	jive_graph graph;

	auto s0 = graph.import(jive::bits::type(32), "s0");
	auto s1 = graph.import(jive::bits::type(32), "s1");

	auto diff = dynamic_cast<jive::output*>(jive_bitdifference(s0, s1));

	graph.export_port(diff, "dummy");

	graph.normalize();
	graph.prune();
	jive::view(graph.root(), stdout);

	assert(diff->node()->operation() == jive::bits::sub_op(32));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitdifference", types_bitstring_arithmetic_test_bitdifference);

static int types_bitstring_arithmetic_test_bitnegate(void)
{
	jive_graph graph;

	auto s0 = graph.import(jive::bits::type(32), "s0");
	auto c0 = jive_bitconstant_unsigned(graph.root(), 32, 3);

	auto neg0 = dynamic_cast<jive::output*>(jive_bitnegate(s0));
	auto neg1 = dynamic_cast<jive::output*>(jive_bitnegate(c0));
	auto neg2 = dynamic_cast<jive::output*>(jive_bitnegate(neg1));

	graph.export_port(neg0, "dummy");
	graph.export_port(neg1, "dummy");
	graph.export_port(neg2, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(neg0->node()->operation() == jive::bits::neg_op(32));
	assert(neg1->node()->operation() == jive::bits::int_constant_op(32, -3));
	assert(neg2->node()->operation() == jive::bits::int_constant_op(32, 3));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitnegate", types_bitstring_arithmetic_test_bitnegate);

static int types_bitstring_arithmetic_test_bitnot(void)
{
	jive_graph graph;

	auto s0 = graph.import(jive::bits::type(32), "s0");
	auto c0 = jive_bitconstant_unsigned(graph.root(), 32, 3);

	auto not0 = dynamic_cast<jive::output*>(jive_bitnot(s0));
	auto not1 = dynamic_cast<jive::output*>(jive_bitnot(c0));
	auto not2 = dynamic_cast<jive::output*>(jive_bitnot(not1));

	graph.export_port(not0, "dummy");
	graph.export_port(not1, "dummy");
	graph.export_port(not2, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(not0->node()->operation() == jive::bits::not_op(32));
	assert(not1->node()->operation() == jive::bits::int_constant_op(32, -4));
	assert(not2->node()->operation() == jive::bits::int_constant_op(32, 3));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitnot", types_bitstring_arithmetic_test_bitnot);

static int types_bitstring_arithmetic_test_bitor(void)
{
	jive_graph graph;

	auto s0 = graph.import(jive::bits::type(32), "s0");
	auto s1 = graph.import(jive::bits::type(32), "s1");

	auto c0 = jive_bitconstant_unsigned(graph.root(), 32, 3);
	auto c1 = jive_bitconstant_unsigned(graph.root(), 32, 5);

	auto or0 = dynamic_cast<jive::output*>(jive_bitor({s0, s1}));
	auto or1 = dynamic_cast<jive::output*>(jive_bitor({c0, c1}));

	graph.export_port(or0, "dummy");
	graph.export_port(or1, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(or0->node()->operation() == jive::bits::or_op(32));
	assert(or1->node()->operation() == jive::bits::uint_constant_op(32, 7));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitor", types_bitstring_arithmetic_test_bitor);

static int types_bitstring_arithmetic_test_bitproduct(void)
{
	jive_graph graph;

	auto s0 = graph.import(jive::bits::type(32), "s0");
	auto s1 = graph.import(jive::bits::type(32), "s1");

	auto c0 = jive_bitconstant_unsigned(graph.root(), 32, 3);
	auto c1 = jive_bitconstant_unsigned(graph.root(), 32, 5);

	auto product0 = dynamic_cast<jive::output*>(jive_bitmultiply({s0, s1}));
	auto product1 = dynamic_cast<jive::output*>(jive_bitmultiply({c0, c1}));

	graph.export_port(product0, "dummy");
	graph.export_port(product1, "dummy");

	graph.normalize();
	graph.prune();
	jive::view(graph.root(), stdout);

	assert(product0->node()->operation() == jive::bits::mul_op(32));
	assert(product1->node()->operation() == jive::bits::uint_constant_op(32, 15));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitproduct", types_bitstring_arithmetic_test_bitproduct);

static int types_bitstring_arithmetic_test_bitshiproduct(void)
{
	jive_graph graph;

	auto s0 = graph.import(jive::bits::type(32), "s0");
	auto s1 = graph.import(jive::bits::type(32), "s1");

	auto shiproduct = dynamic_cast<jive::output*>(jive_bitshiproduct(s0, s1));

	graph.export_port(shiproduct, "dummy");

	graph.normalize();
	graph.prune();
	jive::view(graph.root(), stdout);

	assert(shiproduct->node()->operation() == jive::bits::smulh_op(32));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitshiproduct", types_bitstring_arithmetic_test_bitshiproduct);

static int types_bitstring_arithmetic_test_bitshl(void)
{
	jive_graph graph;

	auto s0 = graph.import(jive::bits::type(32), "s0");
	auto s1 = graph.import(jive::bits::type(32), "s1");

	auto c0 = jive_bitconstant_unsigned(graph.root(), 32, 16);
	auto c1 = jive_bitconstant_unsigned(graph.root(), 32, 2);
	auto c2 = jive_bitconstant_unsigned(graph.root(), 32, 32);

	auto shl0 = dynamic_cast<jive::output*>(jive_bitshl(s0, s1));
	auto shl1 = dynamic_cast<jive::output*>(jive_bitshl(c0, c1));
	auto shl2 = dynamic_cast<jive::output*>(jive_bitshl(c0, c2));

	graph.export_port(shl0, "dummy");
	graph.export_port(shl1, "dummy");
	graph.export_port(shl2, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(shl0->node()->operation() == jive::bits::shl_op(32));
	assert(shl1->node()->operation() == jive::bits::uint_constant_op(32, 64));
	assert(shl2->node()->operation() == jive::bits::uint_constant_op(32, 0));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitshl", types_bitstring_arithmetic_test_bitshl);

static int types_bitstring_arithmetic_test_bitshr(void)
{
	jive_graph graph;

	auto s0 = graph.import(jive::bits::type(32), "s0");
	auto s1 = graph.import(jive::bits::type(32), "s1");

	auto c0 = jive_bitconstant_unsigned(graph.root(), 32, 16);
	auto c1 = jive_bitconstant_unsigned(graph.root(), 32, 2);
	auto c2 = jive_bitconstant_unsigned(graph.root(), 32, 32);

	auto shr0 = dynamic_cast<jive::output*>(jive_bitshr(s0, s1));
	auto shr1 = dynamic_cast<jive::output*>(jive_bitshr(c0, c1));
	auto shr2 = dynamic_cast<jive::output*>(jive_bitshr(c0, c2));

	graph.export_port(shr0, "dummy");
	graph.export_port(shr1, "dummy");
	graph.export_port(shr2, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);
	
	assert(shr0->node()->operation() == jive::bits::shr_op(32));
	assert(shr1->node()->operation() == jive::bits::uint_constant_op(32, 4));
	assert(shr2->node()->operation() == jive::bits::uint_constant_op(32, 0));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitshr", types_bitstring_arithmetic_test_bitshr);

static int types_bitstring_arithmetic_test_bitsmod(void)
{
	jive_graph graph;

	auto s0 = graph.import(jive::bits::type(32), "s0");
	auto s1 = graph.import(jive::bits::type(32), "s1");

	auto c0 = jive_bitconstant_signed(graph.root(), 32, -7);
	auto c1 = jive_bitconstant_signed(graph.root(), 32, 3);

	auto smod0 = dynamic_cast<jive::output*>(jive_bitsmod(s0, s1));
	auto smod1 = dynamic_cast<jive::output*>(jive_bitsmod(c0, c1));

	graph.export_port(smod0, "dummy");
	graph.export_port(smod1, "dummy");

	graph.normalize();
	graph.prune();
	jive::view(graph.root(), stdout);

	assert(smod0->node()->operation() == jive::bits::smod_op(32));
	assert(smod1->node()->operation() == jive::bits::int_constant_op(32, -1));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitsmod", types_bitstring_arithmetic_test_bitsmod);

static int types_bitstring_arithmetic_test_bitsquotient(void)
{
	jive_graph graph;

	auto s0 = graph.import(jive::bits::type(32), "s0");
	auto s1 = graph.import(jive::bits::type(32), "s1");

	auto c0 = jive_bitconstant_signed(graph.root(), 32, 7);
	auto c1 = jive_bitconstant_signed(graph.root(), 32, -3);

	auto squot0 = dynamic_cast<jive::output*>(jive_bitsquotient(s0, s1));
	auto squot1 = dynamic_cast<jive::output*>(jive_bitsquotient(c0, c1));

	graph.export_port(squot0, "dummy");
	graph.export_port(squot1, "dummy");

	graph.normalize();
	graph.prune();
	jive::view(graph.root(), stdout);

	assert(squot0->node()->operation() == jive::bits::sdiv_op(32));
	assert(squot1->node()->operation() == jive::bits::int_constant_op(32, -2));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitsquotient", types_bitstring_arithmetic_test_bitsquotient);

static int types_bitstring_arithmetic_test_bitsum(void)
{
	jive_graph graph;

	auto s0 = graph.import(jive::bits::type(32), "s0");
	auto s1 = graph.import(jive::bits::type(32), "s1");

	auto c0 = jive_bitconstant_unsigned(graph.root(), 32, 3);
	auto c1 = jive_bitconstant_unsigned(graph.root(), 32, 5);

	auto sum0 = dynamic_cast<jive::output*>(jive_bitsum({s0, s1}));
	auto sum1 = dynamic_cast<jive::output*>(jive_bitsum({c0, c1}));

	graph.export_port(sum0, "dummy");
	graph.export_port(sum1, "dummy");

	graph.normalize();
	graph.prune();
	jive::view(graph.root(), stdout);

	assert(sum0->node()->operation() == jive::bits::add_op(32));
	assert(sum1->node()->operation() == jive::bits::int_constant_op(32, 8));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitsum", types_bitstring_arithmetic_test_bitsum);

static int types_bitstring_arithmetic_test_bituhiproduct(void)
{
	jive_graph graph;

	auto s0 = graph.import(jive::bits::type(32), "s0");
	auto s1 = graph.import(jive::bits::type(32), "s1");

	auto uhiproduct = dynamic_cast<jive::output*>(jive_bituhiproduct(s0, s1));

	graph.export_port(uhiproduct, "dummy");

	graph.normalize();
	graph.prune();
	jive::view(graph.root(), stdout);

	assert(uhiproduct->node()->operation() == jive::bits::umulh_op(32));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bituhiproduct", types_bitstring_arithmetic_test_bituhiproduct);

static int types_bitstring_arithmetic_test_bitumod(void)
{
	jive_graph graph;

	auto s0 = graph.import(jive::bits::type(32), "s0");
	auto s1 = graph.import(jive::bits::type(32), "s1");

	auto c0 = jive_bitconstant_unsigned(graph.root(), 32, 7);
	auto c1 = jive_bitconstant_unsigned(graph.root(), 32, 3);

	auto umod0 = dynamic_cast<jive::output*>(jive_bitumod(s0, s1));
	auto umod1 = dynamic_cast<jive::output*>(jive_bitumod(c0, c1));

	graph.export_port(umod0, "dummy");
	graph.export_port(umod1, "dummy");

	graph.normalize();
	graph.prune();
	jive::view(graph.root(), stdout);

	assert(umod0->node()->operation() == jive::bits::umod_op(32));
	assert(umod1->node()->operation() == jive::bits::int_constant_op(32, 1));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitumod", types_bitstring_arithmetic_test_bitumod);

static int types_bitstring_arithmetic_test_bituquotient(void)
{
	jive_graph graph;

	auto s0 = graph.import(jive::bits::type(32), "s0");
	auto s1 = graph.import(jive::bits::type(32), "s1");

	auto c0 = jive_bitconstant_unsigned(graph.root(), 32, 7);
	auto c1 = jive_bitconstant_unsigned(graph.root(), 32, 3);

	auto uquot0 = dynamic_cast<jive::output*>(jive_bituquotient(s0, s1));
	auto uquot1 = dynamic_cast<jive::output*>(jive_bituquotient(c0, c1));

	graph.export_port(uquot0, "dummy");
	graph.export_port(uquot1, "dummy");

	graph.normalize();
	graph.prune();
	jive::view(graph.root(), stdout);

	assert(uquot0->node()->operation() == jive::bits::udiv_op(32));
	assert(uquot1->node()->operation() == jive::bits::int_constant_op(32, 2));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bituquotient", types_bitstring_arithmetic_test_bituquotient);

static int types_bitstring_arithmetic_test_bitxor(void)
{
	jive_graph graph;

	auto s0 = graph.import(jive::bits::type(32), "s0");
	auto s1 = graph.import(jive::bits::type(32), "s1");

	auto c0 = jive_bitconstant_unsigned(graph.root(), 32, 3);
	auto c1 = jive_bitconstant_unsigned(graph.root(), 32, 5);

	auto xor0 = dynamic_cast<jive::output*>(jive_bitxor({s0, s1}));
	auto xor1 = dynamic_cast<jive::output*>(jive_bitxor({c0, c1}));

	graph.export_port(xor0, "dummy");
	graph.export_port(xor1, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(xor0->node()->operation() == jive::bits::xor_op(32));
	assert(xor1->node()->operation() == jive::bits::int_constant_op(32, 6));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitxor", types_bitstring_arithmetic_test_bitxor);

static inline void
expect_static_true(jive::oport * port)
{
	auto output = dynamic_cast<jive::output*>(port);
	const jive::bits::constant_op * op;
	op = dynamic_cast<const jive::bits::constant_op*>(&output->node()->operation());
	assert(op && op->value().nbits() == 1 && op->value().str() == "1");
}

static inline void
expect_static_false(jive::oport * port)
{
	auto output = dynamic_cast<jive::output*>(port);
	const jive::bits::constant_op * op;
	op = dynamic_cast<const jive::bits::constant_op*>(&output->node()->operation());
	assert(op && op->value().nbits() == 1 && op->value().str() == "0");
}

static int types_bitstring_comparison_test_bitequal(void)
{
	jive_graph graph;

	auto s0 = graph.import(jive::bits::type(32), "s0");
	auto s1 = graph.import(jive::bits::type(32), "s1");
	auto c0 = jive_bitconstant_unsigned(graph.root(), 32, 4);
	auto c1 = jive_bitconstant_unsigned(graph.root(), 32, 5);
	auto c2 = jive_bitconstant_undefined(graph.root(), 32);

	auto equal0 = dynamic_cast<jive::output*>(jive_bitequal(s0, s1));
	auto equal1 = jive_bitequal(c0, c0);
	auto equal2 = jive_bitequal(c0, c1);
	auto equal3 = dynamic_cast<jive::output*>(jive_bitequal(c0, c2));

	graph.export_port(equal0, "dummy");
	graph.export_port(equal1, "dummy");
	graph.export_port(equal2, "dummy");
	graph.export_port(equal3, "dummy");
	
	graph.prune();
	jive::view(graph.root(), stdout);

	assert(equal0->node()->operation() == jive::bits::eq_op(32));
	expect_static_true(equal1);
	expect_static_false(equal2);
	assert(equal3->node()->operation() == jive::bits::eq_op(32));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitequal", types_bitstring_comparison_test_bitequal);

static int types_bitstring_comparison_test_bitnotequal(void)
{
	jive_graph graph;

	auto s0 = graph.import(jive::bits::type(32), "s0");
	auto s1 = graph.import(jive::bits::type(32), "s1");
	auto c0 = jive_bitconstant_unsigned(graph.root(), 32, 4);
	auto c1 = jive_bitconstant_unsigned(graph.root(), 32, 5);
	auto c2 = jive_bitconstant_undefined(graph.root(), 32);

	auto nequal0 = dynamic_cast<jive::output*>(jive_bitnotequal(s0, s1));
	auto nequal1 = jive_bitnotequal(c0, c0);
	auto nequal2 = jive_bitnotequal(c0, c1);
	auto nequal3 = dynamic_cast<jive::output*>(jive_bitnotequal(c0, c2));

	graph.export_port(nequal0, "dummy");
	graph.export_port(nequal1, "dummy");
	graph.export_port(nequal2, "dummy");
	graph.export_port(nequal3, "dummy");
	
	graph.prune();
	jive::view(graph.root(), stdout);

	assert(nequal0->node()->operation() == jive::bits::ne_op(32));
	expect_static_false(nequal1);
	expect_static_true(nequal2);
	assert(nequal3->node()->operation() == jive::bits::ne_op(32));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitnotequal", types_bitstring_comparison_test_bitnotequal);

static int types_bitstring_comparison_test_bitsgreater(void)
{
	jive_graph graph;

	auto s0 = graph.import(jive::bits::type(32), "s0");
	auto s1 = graph.import(jive::bits::type(32), "s1");
	auto c0 = jive_bitconstant_signed(graph.root(), 32, 4);
	auto c1 = jive_bitconstant_signed(graph.root(), 32, 5);
	auto c2 = jive_bitconstant_signed(graph.root(), 32, 0x7fffffffL);
	auto c3 = jive_bitconstant_signed(graph.root(), 32, (-0x7fffffffL-1));

	auto sgreater0 = dynamic_cast<jive::output*>(jive_bitsgreater(s0, s1));
	auto sgreater1 = jive_bitsgreater(c0, c1);
	auto sgreater2 = jive_bitsgreater(c1, c0);
	auto sgreater3 = jive_bitsgreater(s0, c2);
	auto sgreater4 = jive_bitsgreater(c3, s1);

	graph.export_port(sgreater0, "dummy");
	graph.export_port(sgreater1, "dummy");
	graph.export_port(sgreater2, "dummy");
	graph.export_port(sgreater3, "dummy");
	graph.export_port(sgreater4, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(sgreater0->node()->operation() == jive::bits::sgt_op(32));
	expect_static_false(sgreater1);
	expect_static_true(sgreater2);
	expect_static_false(sgreater3);
	expect_static_false(sgreater4);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitsgreater", types_bitstring_comparison_test_bitsgreater);

static int types_bitstring_comparison_test_bitsgreatereq(void)
{
	jive_graph graph;

	auto s0 = graph.import(jive::bits::type(32), "s0");
	auto s1 = graph.import(jive::bits::type(32), "s1");
	auto c0 = jive_bitconstant_signed(graph.root(), 32, 4);
	auto c1 = jive_bitconstant_signed(graph.root(), 32, 5);
	auto c2 = jive_bitconstant_signed(graph.root(), 32, 0x7fffffffL);
	auto c3 = jive_bitconstant_signed(graph.root(), 32, (-0x7fffffffL-1));

	auto sgreatereq0 = dynamic_cast<jive::output*>(jive_bitsgreatereq(s0, s1));
	auto sgreatereq1 = jive_bitsgreatereq(c0, c1);
	auto sgreatereq2 = jive_bitsgreatereq(c1, c0);
	auto sgreatereq3 = jive_bitsgreatereq(c0, c0);
	auto sgreatereq4 = jive_bitsgreatereq(c2, s0);
	auto sgreatereq5 = jive_bitsgreatereq(s1, c3);

	graph.export_port(sgreatereq0, "dummy");
	graph.export_port(sgreatereq1, "dummy");
	graph.export_port(sgreatereq2, "dummy");
	graph.export_port(sgreatereq3, "dummy");
	graph.export_port(sgreatereq4, "dummy");
	graph.export_port(sgreatereq5, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(sgreatereq0->node()->operation() == jive::bits::sge_op(32));
	expect_static_false(sgreatereq1);
	expect_static_true(sgreatereq2);
	expect_static_true(sgreatereq3);
	expect_static_true(sgreatereq4);
	expect_static_true(sgreatereq5);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitsgreatereq", types_bitstring_comparison_test_bitsgreatereq);

static int types_bitstring_comparison_test_bitsless(void)
{
	jive_graph graph;

	auto s0 = graph.import(jive::bits::type(32), "s0");
	auto s1 = graph.import(jive::bits::type(32), "s1");
	auto c0 = jive_bitconstant_signed(graph.root(), 32, 4);
	auto c1 = jive_bitconstant_signed(graph.root(), 32, 5);
	auto c2 = jive_bitconstant_signed(graph.root(), 32, 0x7fffffffL);
	auto c3 = jive_bitconstant_signed(graph.root(), 32, (-0x7fffffffL-1));

	auto sless0 = dynamic_cast<jive::output*>(jive_bitsless(s0, s1));
	auto sless1 = jive_bitsless(c0, c1);
	auto sless2 = jive_bitsless(c1, c0);
	auto sless3 = jive_bitsless(c2, s0);
	auto sless4 = jive_bitsless(s1, c3);

	graph.export_port(sless0, "dummy");
	graph.export_port(sless1, "dummy");
	graph.export_port(sless2, "dummy");
	graph.export_port(sless3, "dummy");
	graph.export_port(sless4, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(sless0->node()->operation() == jive::bits::slt_op(32));
	expect_static_true(sless1);
	expect_static_false(sless2);
	expect_static_false(sless3);
	expect_static_false(sless4);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitsless", types_bitstring_comparison_test_bitsless);

static int types_bitstring_comparison_test_bitslesseq(void)
{
	jive_graph graph;

	auto s0 = graph.import(jive::bits::type(32), "s0");
	auto s1 = graph.import(jive::bits::type(32), "s1");
	auto c0 = jive_bitconstant_signed(graph.root(), 32, 4);
	auto c1 = jive_bitconstant_signed(graph.root(), 32, 5);
	auto c2 = jive_bitconstant_signed(graph.root(), 32, 0x7fffffffL);
	auto c3 = jive_bitconstant_signed(graph.root(), 32, (-0x7fffffffL-1));

	auto slesseq0 = dynamic_cast<jive::output*>(jive_bitslesseq(s0, s1));
	auto slesseq1 = jive_bitslesseq(c0, c1);
	auto slesseq2 = jive_bitslesseq(c0, c0);
	auto slesseq3 = jive_bitslesseq(c1, c0);
	auto slesseq4 = jive_bitslesseq(s0, c2);
	auto slesseq5 = jive_bitslesseq(c3, s1);

	graph.export_port(slesseq0, "dummy");
	graph.export_port(slesseq1, "dummy");
	graph.export_port(slesseq2, "dummy");
	graph.export_port(slesseq3, "dummy");
	graph.export_port(slesseq4, "dummy");
	graph.export_port(slesseq5, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(slesseq0->node()->operation() == jive::bits::sle_op(32));
	expect_static_true(slesseq1);
	expect_static_true(slesseq2);
	expect_static_false(slesseq3);
	expect_static_true(slesseq4);
	expect_static_true(slesseq5);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitslesseq", types_bitstring_comparison_test_bitslesseq);

static int types_bitstring_comparison_test_bitugreater(void)
{
	jive_graph graph;

	auto s0 = graph.import(jive::bits::type(32), "s0");
	auto s1 = graph.import(jive::bits::type(32), "s1");
	auto c0 = jive_bitconstant_unsigned(graph.root(), 32, 4);
	auto c1 = jive_bitconstant_unsigned(graph.root(), 32, 5);
	auto c2 = jive_bitconstant_unsigned(graph.root(), 32, (0xffffffffUL));
	auto c3 = jive_bitconstant_unsigned(graph.root(), 32, 0);

	auto ugreater0 = dynamic_cast<jive::output*>(jive_bitugreater(s0, s1));
	auto ugreater1 = jive_bitugreater(c0, c1);
	auto ugreater2 = jive_bitugreater(c1, c0);
	auto ugreater3 = jive_bitugreater(s0, c2);
	auto ugreater4 = jive_bitugreater(c3, s1);

	graph.export_port(ugreater0, "dummy");
	graph.export_port(ugreater1, "dummy");
	graph.export_port(ugreater2, "dummy");
	graph.export_port(ugreater3, "dummy");
	graph.export_port(ugreater4, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(ugreater0->node()->operation() == jive::bits::ugt_op(32));
	expect_static_false(ugreater1);
	expect_static_true(ugreater2);
	expect_static_false(ugreater3);
	expect_static_false(ugreater4);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitugreater", types_bitstring_comparison_test_bitugreater);

static int types_bitstring_comparison_test_bitugreatereq(void)
{
	jive_graph graph;

	auto s0 = graph.import(jive::bits::type(32), "s0");
	auto s1 = graph.import(jive::bits::type(32), "s1");
	auto c0 = jive_bitconstant_signed(graph.root(), 32, 4);
	auto c1 = jive_bitconstant_signed(graph.root(), 32, 5);
	auto c2 = jive_bitconstant_signed(graph.root(), 32, (0xffffffffUL));
	auto c3 = jive_bitconstant_signed(graph.root(), 32, 0);

	auto ugreatereq0 = dynamic_cast<jive::output*>(jive_bitugreatereq(s0, s1));
	auto ugreatereq1 = jive_bitugreatereq(c0, c1);
	auto ugreatereq2 = jive_bitugreatereq(c1, c0);
	auto ugreatereq3 = jive_bitugreatereq(c0, c0);
	auto ugreatereq4 = jive_bitugreatereq(c2, s0);
	auto ugreatereq5 = jive_bitugreatereq(s1, c3);

	graph.export_port(ugreatereq0, "dummy");
	graph.export_port(ugreatereq1, "dummy");
	graph.export_port(ugreatereq2, "dummy");
	graph.export_port(ugreatereq3, "dummy");
	graph.export_port(ugreatereq4, "dummy");
	graph.export_port(ugreatereq5, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(ugreatereq0->node()->operation() == jive::bits::uge_op(32));
	expect_static_false(ugreatereq1);
	expect_static_true(ugreatereq2);
	expect_static_true(ugreatereq3);
	expect_static_true(ugreatereq4);
	expect_static_true(ugreatereq5);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitugreatereq", types_bitstring_comparison_test_bitugreatereq);

static int types_bitstring_comparison_test_bituless(void)
{
	jive_graph graph;

	auto s0 = graph.import(jive::bits::type(32), "s0");
	auto s1 = graph.import(jive::bits::type(32), "s1");
	auto c0 = jive_bitconstant_signed(graph.root(), 32, 4);
	auto c1 = jive_bitconstant_signed(graph.root(), 32, 5);
	auto c2 = jive_bitconstant_signed(graph.root(), 32, (0xffffffffUL));
	auto c3 = jive_bitconstant_signed(graph.root(), 32, 0);

	auto uless0 = dynamic_cast<jive::output*>(jive_bituless(s0, s1));
	auto uless1 = jive_bituless(c0, c1);
	auto uless2 = jive_bituless(c1, c0);
	auto uless3 = jive_bituless(c2, s0);
	auto uless4 = jive_bituless(s1, c3);

	graph.export_port(uless0, "dummy");
	graph.export_port(uless1, "dummy");
	graph.export_port(uless2, "dummy");
	graph.export_port(uless3, "dummy");
	graph.export_port(uless4, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(uless0->node()->operation() == jive::bits::ult_op(32));
	expect_static_true(uless1);
	expect_static_false(uless2);
	expect_static_false(uless3);
	expect_static_false(uless4);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bituless", types_bitstring_comparison_test_bituless);

static int types_bitstring_comparison_test_bitulesseq(void)
{
	jive_graph graph;

	auto s0 = graph.import(jive::bits::type(32), "s0");
	auto s1 = graph.import(jive::bits::type(32), "s1");
	auto c0 = jive_bitconstant_signed(graph.root(), 32, 4);
	auto c1 = jive_bitconstant_signed(graph.root(), 32, 5);
	auto c2 = jive_bitconstant_signed(graph.root(), 32, (0xffffffffUL));
	auto c3 = jive_bitconstant_signed(graph.root(), 32, 0);

	auto ulesseq0 = dynamic_cast<jive::output*>(jive_bitulesseq(s0, s1));
	auto ulesseq1 = jive_bitulesseq(c0, c1);
	auto ulesseq2 = jive_bitulesseq(c0, c0);
	auto ulesseq3 = jive_bitulesseq(c1, c0);
	auto ulesseq4 = jive_bitulesseq(s0, c2);
	auto ulesseq5 = jive_bitulesseq(c3, s1);

	graph.export_port(ulesseq0, "dummy");
	graph.export_port(ulesseq1, "dummy");
	graph.export_port(ulesseq2, "dummy");
	graph.export_port(ulesseq3, "dummy");
	graph.export_port(ulesseq4, "dummy");
	graph.export_port(ulesseq5, "dummy");

	graph.prune();
	jive::view(graph.root(), stdout);

	assert(ulesseq0->node()->operation() == jive::bits::ule_op(32));
	expect_static_true(ulesseq1);
	expect_static_true(ulesseq2);
	expect_static_false(ulesseq3);
	expect_static_true(ulesseq4);
	expect_static_true(ulesseq5);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitulesseq", types_bitstring_comparison_test_bitulesseq);

static int types_bitstring_test_arithmetic(void)
{
	jive_graph graph;

	auto c0 = jive_bitconstant(graph.root(), 4, "1100");
	auto c1 = jive_bitconstant(graph.root(), 4, "0001");

	jive_bitdifference(c0, c1);
	jive_bitshiproduct(c0, c1);
	jive_bituhiproduct(c0, c1);
	jive_bituquotient(c0, c1);
	jive_bitsquotient(c0, c1);
	jive_bitumod(c0, c1);
	jive_bitsmod(c0, c1);
	jive_bitshl(c0, c1);
	jive_bitshr(c0, c1);
	jive_bitashr(c0, c1);

	jive::view(graph.root(), stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/test-arithmetic", types_bitstring_test_arithmetic);

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
	jive_graph graph;

	auto b1 = dynamic_cast<jive::output*>(jive_bitconstant(graph.root(), 8, "00110011"));
	auto b2 = dynamic_cast<jive::output*>(jive_bitconstant_unsigned(graph.root(), 8, 204));
	auto b3 = dynamic_cast<jive::output*>(jive_bitconstant_signed(graph.root(), 8, 204));
	auto b4 = dynamic_cast<jive::output*>(jive_bitconstant(graph.root(), 9, "001100110"));
	
	assert(b1->node()->operation() == jive::bits::uint_constant_op(8, 204));
	assert(b1->node()->operation() == jive::bits::int_constant_op(8, -52));

	assert(b1->node() == b2->node());
	assert(b1->node() == b3->node());
	
	assert(b1->node()->operation() == jive::bits::uint_constant_op(8, 204));
	assert(b1->node()->operation() == jive::bits::int_constant_op(8, -52));

	assert(b4->node()->operation() == jive::bits::uint_constant_op(9, 204));
	assert(b4->node()->operation() == jive::bits::int_constant_op(9, 204));

	auto plus_one_128 = dynamic_cast<jive::output*>(
		jive_bitconstant(graph.root(), 128, ONE_64 ZERO_64));
	assert(plus_one_128->node()->operation() == jive::bits::uint_constant_op(128, 1));
	assert(plus_one_128->node()->operation() == jive::bits::int_constant_op(128, 1));

	auto minus_one_128 = dynamic_cast<jive::output*>(
		jive_bitconstant(graph.root(), 128, MONE_64 MONE_64));
	assert(minus_one_128->node()->operation() == jive::bits::int_constant_op(128, -1));

	jive::view(graph.root(), stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/test-constant", types_bitstring_test_constant);

static int types_bitstring_test_normalize(void)
{
	jive_graph graph;

	jive::bits::type bits32(32);
	const jive::base::type * tmparray11[] = {&bits32};
	jive_lambda * lambda = jive_lambda_begin(graph.root(), {{&bits32, "arg"}}, {{&bits32, "r"}});

	auto c0 = jive_bitconstant_unsigned(lambda->region, 32, 3);
	auto c1 = jive_bitconstant_unsigned(lambda->region, 32, 4);
	
	auto  sum_nf = graph.node_normal_form(typeid(jive::bits::add_op));
	assert(sum_nf);
	sum_nf->set_mutable(false);

	auto sum0 = jive_bitsum({lambda->arguments[0], c0});
	assert(sum0->node()->operation() == jive::bits::add_op(32));
	assert(sum0->node()->noperands() == 2);

	auto sum1 = jive_bitsum({sum0, c1});
	assert(sum1->node()->operation() == jive::bits::add_op(32));
	assert(sum1->node()->noperands() == 2);

	auto lambda_node = dynamic_cast<jive::structural_node*>(
		jive_lambda_end(lambda, 1, tmparray11, &sum1)->node());

	jive::iport * retval = lambda_node->subregion(0)->result(0);
	graph.export_port(lambda_node->output(0), "dummy");

	sum_nf->set_mutable(true);
	graph.normalize();
	graph.prune();

	auto expected_sum = retval->origin();
	assert(expected_sum->node()->operation() == jive::bits::add_op(32));
	assert(expected_sum->node()->noperands() == 2);
	auto op1 = expected_sum->node()->input(0)->origin();
	auto op2 = expected_sum->node()->input(1)->origin();
	if (!dynamic_cast<const jive::bits::constant_op *>(&op1->node()->operation())) {
		auto tmp = op1; op1 = op2; op2 = tmp;
	}
	/* FIXME: the graph traversers are currently broken, that is why it won't normalize */
//	assert(op1->node()->operation() == jive::bits::int_constant_op(32, 3 + 4));
//	assert(op2 == lambda_node->subregion(0)->argument(0));

	jive::view(graph.root(), stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/test-normalize", types_bitstring_test_normalize);

static void
assert_constant(jive::oport * bitstr, size_t nbits, const char bits[])
{
	auto tmp = dynamic_cast<jive::output*>(bitstr);
	auto op = dynamic_cast<const jive::bits::constant_op &>(tmp->node()->operation());
	
	assert(op.value() == jive::bits::value_repr(std::string(bits, nbits).c_str()));
}

static int types_bitstring_test_reduction(void)
{
	jive_graph graph;

	auto a = jive_bitconstant(graph.root(), 4, "1100");
	auto b = jive_bitconstant(graph.root(), 4, "1010");

	assert_constant(jive_bitand({a, b}), 4, "1000");
	assert_constant(jive_bitor({a, b}), 4, "1110");
	assert_constant(jive_bitxor({a, b}), 4, "0110");
	assert_constant(jive_bitsum({a, b}), 4, "0001");
	assert_constant(jive_bitmultiply({a, b}), 4, "1111");
	assert_constant(jive_bitconcat({a, b}), 8, "11001010");
	assert_constant(jive_bitnegate(a), 4, "1011");
	assert_constant(jive_bitnegate(b), 4, "1101");
	
	graph.prune();
	
	auto x = graph.import(jive::bits::type(16), "x");
	auto y = graph.import(jive::bits::type(16), "y");
	
	{
		auto concat = jive_bitconcat({x, y});
		auto slice = jive_bitslice(concat, 8, 24);
		jive::node * node = dynamic_cast<jive::output*>(slice)->node();
		assert(dynamic_cast<const jive::bits::concat_op *>(&node->operation()));
		assert(node->ninputs() == 2);
		assert(dynamic_cast<const jive::bits::slice_op *>(
			&dynamic_cast<jive::output*>(node->input(0)->origin())->node()->operation()));
		assert(dynamic_cast<const jive::bits::slice_op *>(
			&dynamic_cast<jive::output*>(node->input(1)->origin())->node()->operation()));
		
		const jive::bits::slice_op * attrs;
		attrs = dynamic_cast<const jive::bits::slice_op*>(
			&dynamic_cast<jive::output*>(node->input(0)->origin())->node()->operation());
		assert( (attrs->low() == 8) && (attrs->high() == 16) );
		attrs = dynamic_cast<const jive::bits::slice_op*>(
			&dynamic_cast<jive::output*>(node->input(1)->origin())->node()->operation());
		assert( (attrs->low() == 0) && (attrs->high() == 8) );
		
		assert(dynamic_cast<jive::output*>(node->input(0)->origin())->node()->input(0)->origin() == x);
		assert(dynamic_cast<jive::output*>(node->input(1)->origin())->node()->input(0)->origin() == y);
	}
	
	{
		auto slice1 = jive_bitslice(x, 0, 8);
		auto slice2 = jive_bitslice(x, 8, 16);
		auto concat = jive_bitconcat({slice1, slice2});
		assert(concat == x);
	}

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/test-reduction", types_bitstring_test_reduction);

static int types_bitstring_test_slice_concat(void)
{
	jive_graph graph;
	
	auto base_const1 = jive_bitconstant(graph.root(), 8, "00110111");
	auto base_const2 = jive_bitconstant(graph.root(), 8, "11001000");
	
	auto base_x = graph.import(jive::bits::type(8), "x");
	auto base_y = graph.import(jive::bits::type(8), "y");
	auto base_z = graph.import(jive::bits::type(8), "z");
	
	{
		/* slice of constant */
		auto a = dynamic_cast<jive::output*>(jive_bitslice(base_const1, 2, 6));
		
		const jive::bits::constant_op & op =
			dynamic_cast<const jive::bits::constant_op &>(a->node()->operation());
		assert(op.value() == jive::bits::value_repr("1101"));
	}
	
	{
		/* slice of slice */
		auto a = jive_bitslice(base_x, 2, 6);
		auto b = dynamic_cast<jive::output*>(jive_bitslice(a, 1, 3));

		assert(dynamic_cast<const jive::bits::slice_op *>(&b->node()->operation()));
		const jive::bits::slice_op * attrs;
		attrs = dynamic_cast<const jive::bits::slice_op *>(&b->node()->operation());
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
		
		assert(static_cast<const jive::bits::type*>(&b->type())->nbits() == 8);
		
		assert(b == base_x);
	}
	
	{
		/* concat flattening */
		auto a = jive_bitconcat({base_x, base_y});
		auto b = dynamic_cast<jive::output*>(jive_bitconcat({a, base_z}));
		
		assert(dynamic_cast<const jive::bits::concat_op *>(&b->node()->operation()));
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
		auto a = dynamic_cast<jive::output*>(jive_bitconcat({base_const1, base_const2}));
		
		const jive::bits::constant_op & op =
			dynamic_cast<const jive::bits::constant_op &>(a->node()->operation());
		assert(op.value() == jive::bits::value_repr("0011011111001000"));
	}
	
	{
		/* CSE */
		auto b = jive_bitconstant(graph.root(), 8, "00110111");
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

JIVE_UNIT_TEST_REGISTER("types/bitstring/test-slice-concat", types_bitstring_test_slice_concat);

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
	for (size_t r = 0; r < 10; r++) {
		assert(jive::bits::value_repr(bs[r]).lnot() == bitstring_not[r]);
		for (size_t c = 0; c < 10; c++) {
			assert(jive::bits::value_repr(bs[r]).land(bs[c]) == bitstring_and[r][c]);
			assert(jive::bits::value_repr(bs[r]).lor(bs[c]) == bitstring_or[r][c]);
			assert(jive::bits::value_repr(bs[r]).lxor(bs[c]) == bitstring_xor[r][c]);

			assert(jive::bits::value_repr(bs[r]).ult(bs[c]) == uless[r][c]);
			assert(jive::bits::value_repr(bs[r]).slt(bs[c]) == sless[r][c]);

			assert(jive::bits::value_repr(bs[r]).ule(bs[c]) == ulesseq[r][c]);
			assert(jive::bits::value_repr(bs[r]).sle(bs[c]) == slesseq[r][c]);

			assert(jive::bits::value_repr(bs[r]).eq(bs[c]) == equal[r][c]);
			assert(jive::bits::value_repr(bs[r]).ne(bs[c]) == notequal[r][c]);

			assert(jive::bits::value_repr(bs[r]).uge(bs[c]) == ugreatereq[r][c]);
			assert(jive::bits::value_repr(bs[r]).sge(bs[c]) == sgreatereq[r][c]);

			assert(jive::bits::value_repr(bs[r]).ugt(bs[c]) == ugreater[r][c]);
			assert(jive::bits::value_repr(bs[r]).sgt(bs[c]) == sgreater[r][c]);
		}
	}

	assert(jive::bits::value_repr("000110").to_uint() == 24);
	assert(jive::bits::value_repr("00011").to_int() == -8);

	for(ssize_t r = -4; r < 5; r++){
		jive::bits::value_repr rbits(32, r);

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
			jive::bits::value_repr cbits(32, c);

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

JIVE_UNIT_TEST_REGISTER("types/bitstring/test-value-representation", types_bitstring_test_value_representation);
