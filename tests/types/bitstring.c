/*
 * Copyright 2010 2011 2012 2013 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>
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
#include <jive/vsdg/node-private.h>


static int types_bitstring_arithmetic_test_bitand(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph->root_region, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph->root_region, 32, "s1");

	jive::output * c0 = jive_bitconstant_unsigned(graph->root_region, 32, 3);
	jive::output * c1 = jive_bitconstant_unsigned(graph->root_region, 32, 5);
	jive::output * tmparray1[] = {s0, s1};

	jive::output * and0 = jive_bitand(2, tmparray1);
	jive::output * tmparray2[] = {c0, c1};
	jive::output * and1 = jive_bitand(2, tmparray2);

	jive_graph_export(graph, and0);
	jive_graph_export(graph, and1);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(and0->node()->operation() == jive::bits::and_op(32));
	assert(and1->node()->operation() == jive::bits::int_constant_op(32, +1));

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitand", types_bitstring_arithmetic_test_bitand);

static int types_bitstring_arithmetic_test_bitashr(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph->root_region, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph->root_region, 32, "s1");

	jive::output * c0 = jive_bitconstant_unsigned(graph->root_region, 32, 16);
	jive::output * c1 = jive_bitconstant_signed(graph->root_region, 32, -16);
	jive::output * c2 = jive_bitconstant_unsigned(graph->root_region, 32, 2);
	jive::output * c3 = jive_bitconstant_unsigned(graph->root_region, 32, 32);

	jive::output * ashr0 = jive_bitashr(s0, s1);
	jive::output * ashr1 = jive_bitashr(c0, c2);
	jive::output * ashr2 = jive_bitashr(c0, c3);
	jive::output * ashr3 = jive_bitashr(c1, c2);
	jive::output * ashr4 = jive_bitashr(c1, c3);

	jive_graph_export(graph, ashr0);
	jive_graph_export(graph, ashr1);
	jive_graph_export(graph, ashr2);
	jive_graph_export(graph, ashr3);
	jive_graph_export(graph, ashr4);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(ashr0->node()->operation() == jive::bits::ashr_op(32));
	assert(ashr1->node()->operation() == jive::bits::int_constant_op(32, 4));
	assert(ashr2->node()->operation() == jive::bits::int_constant_op(32, 0));
	assert(ashr3->node()->operation() == jive::bits::int_constant_op(32, -4));
	assert(ashr4->node()->operation() == jive::bits::int_constant_op(32, -1));

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitashr", types_bitstring_arithmetic_test_bitashr);

static int types_bitstring_arithmetic_test_bitdifference(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph->root_region, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph->root_region, 32, "s1");

	jive::output * diff = jive_bitdifference(s0, s1);

	jive_graph_export(graph, diff);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(diff->node()->operation() == jive::bits::sub_op(32));

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitdifference", types_bitstring_arithmetic_test_bitdifference);

static int types_bitstring_arithmetic_test_bitnegate(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph->root_region, 32, "s0");
	jive::output * c0 = jive_bitconstant_unsigned(graph->root_region, 32, 3);

	jive::output * neg0 = jive_bitnegate(s0);
	jive::output * neg1 = jive_bitnegate(c0);
	jive::output * neg2 = jive_bitnegate(neg1);

	jive_graph_export(graph, neg0);
	jive_graph_export(graph, neg1);
	jive_graph_export(graph, neg2);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(neg0->node()->operation() == jive::bits::neg_op(32));
	assert(neg1->node()->operation() == jive::bits::int_constant_op(32, -3));
	assert(neg2->node()->operation() == jive::bits::int_constant_op(32, 3));

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitnegate", types_bitstring_arithmetic_test_bitnegate);

static int types_bitstring_arithmetic_test_bitnot(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph->root_region, 32, "s0");
	jive::output * c0 = jive_bitconstant_unsigned(graph->root_region, 32, 3);

	jive::output * not0 = jive_bitnot(s0);
	jive::output * not1 = jive_bitnot(c0);
	jive::output * not2 = jive_bitnot(not1);

	jive_graph_export(graph, not0);
	jive_graph_export(graph, not1);
	jive_graph_export(graph, not2);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(not0->node()->operation() == jive::bits::not_op(32));
	assert(not1->node()->operation() == jive::bits::int_constant_op(32, -4));
	assert(not2->node()->operation() == jive::bits::int_constant_op(32, 3));

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitnot", types_bitstring_arithmetic_test_bitnot);

static int types_bitstring_arithmetic_test_bitor(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph->root_region, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph->root_region, 32, "s1");

	jive::output * c0 = jive_bitconstant_unsigned(graph->root_region, 32, 3);
	jive::output * c1 = jive_bitconstant_unsigned(graph->root_region, 32, 5);
	jive::output * tmparray1[] = {s0, s1};

	jive::output * or0 = jive_bitor(2, tmparray1);
	jive::output * tmparray2[] = {c0, c1};
	jive::output * or1 = jive_bitor(2, tmparray2);

	jive_graph_export(graph, or0);
	jive_graph_export(graph, or1);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(or0->node()->operation() == jive::bits::or_op(32));
	assert(or1->node()->operation() == jive::bits::uint_constant_op(32, 7));

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitor", types_bitstring_arithmetic_test_bitor);

static int types_bitstring_arithmetic_test_bitproduct(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph->root_region, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph->root_region, 32, "s1");

	jive::output * c0 = jive_bitconstant_unsigned(graph->root_region, 32, 3);
	jive::output * c1 = jive_bitconstant_unsigned(graph->root_region, 32, 5);
	jive::output * tmparray1[] = {s0,
		s1};

	jive::output * product0 = jive_bitmultiply(2, tmparray1);
	jive::output * tmparray2[] = {c0, c1};
	jive::output * product1 = jive_bitmultiply(2, tmparray2);

	jive_graph_export(graph, product0);
	jive_graph_export(graph, product1);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(product0->node()->operation() == jive::bits::mul_op(32));
	assert(product1->node()->operation() == jive::bits::uint_constant_op(32, 15));

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitproduct", types_bitstring_arithmetic_test_bitproduct);

static int types_bitstring_arithmetic_test_bitshiproduct(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph->root_region, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph->root_region, 32, "s1");

	jive::output * shiproduct = jive_bitshiproduct(s0, s1);

	jive_graph_export(graph, shiproduct);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(shiproduct->node()->operation() == jive::bits::smulh_op(32));

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitshiproduct", types_bitstring_arithmetic_test_bitshiproduct);

static int types_bitstring_arithmetic_test_bitshl(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph->root_region, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph->root_region, 32, "s1");

	jive::output * c0 = jive_bitconstant_unsigned(graph->root_region, 32, 16);
	jive::output * c1 = jive_bitconstant_unsigned(graph->root_region, 32, 2);
	jive::output * c2 = jive_bitconstant_unsigned(graph->root_region, 32, 32);

	jive::output * shl0 = jive_bitshl(s0, s1);
	jive::output * shl1 = jive_bitshl(c0, c1);
	jive::output * shl2 = jive_bitshl(c0, c2);

	jive_graph_export(graph, shl0);
	jive_graph_export(graph, shl1);
	jive_graph_export(graph, shl2);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(shl0->node()->operation() == jive::bits::shl_op(32));
	assert(shl1->node()->operation() == jive::bits::uint_constant_op(32, 64));
	assert(shl2->node()->operation() == jive::bits::uint_constant_op(32, 0));

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitshl", types_bitstring_arithmetic_test_bitshl);

static int types_bitstring_arithmetic_test_bitshr(void)
{
	setlocale(LC_ALL, "");
	
	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph->root_region, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph->root_region, 32, "s1");

	jive::output * c0 = jive_bitconstant_unsigned(graph->root_region, 32, 16);
	jive::output * c1 = jive_bitconstant_unsigned(graph->root_region, 32, 2);
	jive::output * c2 = jive_bitconstant_unsigned(graph->root_region, 32, 32);

	jive::output * shr0 = jive_bitshr(s0, s1);
	jive::output * shr1 = jive_bitshr(c0, c1);
	jive::output * shr2 = jive_bitshr(c0, c2);

	jive_graph_export(graph, shr0);
	jive_graph_export(graph, shr1);
	jive_graph_export(graph, shr2);

	jive_graph_prune(graph);
	jive_view(graph, stdout);
	
	assert(shr0->node()->operation() == jive::bits::shr_op(32));
	assert(shr1->node()->operation() == jive::bits::uint_constant_op(32, 4));
	assert(shr2->node()->operation() == jive::bits::uint_constant_op(32, 0));

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitshr", types_bitstring_arithmetic_test_bitshr);

static int types_bitstring_arithmetic_test_bitsmod(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph->root_region, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph->root_region, 32, "s1");

	jive::output * c0 = jive_bitconstant_signed(graph->root_region, 32, -7);
	jive::output * c1 = jive_bitconstant_signed(graph->root_region, 32, 3);

	jive::output * smod0 = jive_bitsmod(s0, s1);
	jive::output * smod1 = jive_bitsmod(c0, c1);

	jive_graph_export(graph, smod0);
	jive_graph_export(graph, smod1);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(smod0->node()->operation() == jive::bits::smod_op(32));
	assert(smod1->node()->operation() == jive::bits::int_constant_op(32, -1));

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitsmod", types_bitstring_arithmetic_test_bitsmod);

static int types_bitstring_arithmetic_test_bitsquotient(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph->root_region, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph->root_region, 32, "s1");

	jive::output * c0 = jive_bitconstant_signed(graph->root_region, 32, 7);
	jive::output * c1 = jive_bitconstant_signed(graph->root_region, 32, -3);

	jive::output * squot0 = jive_bitsquotient(s0, s1);
	jive::output * squot1 = jive_bitsquotient(c0, c1);

	jive_graph_export(graph, squot0);
	jive_graph_export(graph, squot1);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(squot0->node()->operation() == jive::bits::sdiv_op(32));
	assert(squot1->node()->operation() == jive::bits::int_constant_op(32, -2));

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitsquotient", types_bitstring_arithmetic_test_bitsquotient);

static int types_bitstring_arithmetic_test_bitsum(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph->root_region, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph->root_region, 32, "s1");

	jive::output * c0 = jive_bitconstant_unsigned(graph->root_region, 32, 3);
	jive::output * c1 = jive_bitconstant_unsigned(graph->root_region, 32, 5);

	jive::output * tmparray1[] = {s0, s1};
	jive::output * sum0 = jive_bitsum(2, tmparray1);
	jive::output * tmparray2[] = {c0, c1};
	jive::output * sum1 = jive_bitsum(2, tmparray2);

	jive_graph_export(graph, sum0);
	jive_graph_export(graph, sum1);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(sum0->node()->operation() == jive::bits::add_op(32));
	assert(sum1->node()->operation() == jive::bits::int_constant_op(32, 8));

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitsum", types_bitstring_arithmetic_test_bitsum);

static int types_bitstring_arithmetic_test_bituhiproduct(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph->root_region, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph->root_region, 32, "s1");

	jive::output * uhiproduct = jive_bituhiproduct(s0, s1);

	jive_graph_export(graph, uhiproduct);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(uhiproduct->node()->operation() == jive::bits::umulh_op(32));

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bituhiproduct", types_bitstring_arithmetic_test_bituhiproduct);

static int types_bitstring_arithmetic_test_bitumod(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph->root_region, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph->root_region, 32, "s1");

	jive::output * c0 = jive_bitconstant_unsigned(graph->root_region, 32, 7);
	jive::output * c1 = jive_bitconstant_unsigned(graph->root_region, 32, 3);

	jive::output * umod0 = jive_bitumod(s0, s1);
	jive::output * umod1 = jive_bitumod(c0, c1);

	jive_graph_export(graph, umod0);
	jive_graph_export(graph, umod1);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(umod0->node()->operation() == jive::bits::umod_op(32));
	assert(umod1->node()->operation() == jive::bits::int_constant_op(32, 1));

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitumod", types_bitstring_arithmetic_test_bitumod);

static int types_bitstring_arithmetic_test_bituquotient(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph->root_region, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph->root_region, 32, "s1");

	jive::output * c0 = jive_bitconstant_unsigned(graph->root_region, 32, 7);
	jive::output * c1 = jive_bitconstant_unsigned(graph->root_region, 32, 3);

	jive::output * uquot0 = jive_bituquotient(s0, s1);
	jive::output * uquot1 = jive_bituquotient(c0, c1);

	jive_graph_export(graph, uquot0);
	jive_graph_export(graph, uquot1);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(uquot0->node()->operation() == jive::bits::udiv_op(32));
	assert(uquot1->node()->operation() == jive::bits::int_constant_op(32, 2));

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bituquotient", types_bitstring_arithmetic_test_bituquotient);

static int types_bitstring_arithmetic_test_bitxor(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph->root_region, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph->root_region, 32, "s1");

	jive::output * c0 = jive_bitconstant_unsigned(graph->root_region, 32, 3);
	jive::output * c1 = jive_bitconstant_unsigned(graph->root_region, 32, 5);

	jive::output * tmparray1[] = {s0, s1};
	jive::output * xor0 = jive_bitxor(2, tmparray1);
	jive::output * tmparray2[] = {c0, c1};
	jive::output * xor1 = jive_bitxor(2, tmparray2);

	jive_graph_export(graph, xor0);
	jive_graph_export(graph, xor1);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(xor0->node()->operation() == jive::bits::xor_op(32));
	assert(xor1->node()->operation() == jive::bits::int_constant_op(32, 6));

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitxor", types_bitstring_arithmetic_test_bitxor);

static inline void
expect_static_true(jive::output * output)
{
	const jive::bits::constant_op * op;
	op = dynamic_cast<const jive::bits::constant_op*>(&output->node()->operation());
	assert(op && op->value().nbits() == 1 && op->value().str() == "1");
}

static inline void
expect_static_false(jive::output * output)
{
	const jive::bits::constant_op * op;
	op = dynamic_cast<const jive::bits::constant_op*>(&output->node()->operation());
	assert(op && op->value().nbits() == 1 && op->value().str() == "0");
}

static int types_bitstring_comparison_test_bitequal(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph->root_region, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph->root_region, 32, "s1");
	jive::output * c0 = jive_bitconstant_unsigned(graph->root_region, 32, 4);
	jive::output * c1 = jive_bitconstant_unsigned(graph->root_region, 32, 5);
	jive::output * c2 = jive_bitconstant_undefined(graph->root_region, 32);

	jive::output * equal0 = jive_bitequal(s0, s1);
	jive::output * equal1 = jive_bitequal(c0, c0);
	jive::output * equal2 = jive_bitequal(c0, c1);
	jive::output * equal3 = jive_bitequal(c0, c2);

	jive_graph_export(graph, equal0);
	jive_graph_export(graph, equal1);
	jive_graph_export(graph, equal2);
	jive_graph_export(graph, equal3);
	
	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(equal0->node()->operation() == jive::bits::eq_op(32));
	expect_static_true(equal1);
	expect_static_false(equal2);
	assert(equal3->node()->operation() == jive::bits::eq_op(32));

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitequal", types_bitstring_comparison_test_bitequal);

static int types_bitstring_comparison_test_bitnotequal(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph->root_region, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph->root_region, 32, "s1");
	jive::output * c0 = jive_bitconstant_unsigned(graph->root_region, 32, 4);
	jive::output * c1 = jive_bitconstant_unsigned(graph->root_region, 32, 5);
	jive::output * c2 = jive_bitconstant_undefined(graph->root_region, 32);

	jive::output * nequal0 = jive_bitnotequal(s0, s1);
	jive::output * nequal1 = jive_bitnotequal(c0, c0);
	jive::output * nequal2 = jive_bitnotequal(c0, c1);
	jive::output * nequal3 = jive_bitnotequal(c0, c2);

	jive_graph_export(graph, nequal0);
	jive_graph_export(graph, nequal1);
	jive_graph_export(graph, nequal2);
	jive_graph_export(graph, nequal3);
	
	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(nequal0->node()->operation() == jive::bits::ne_op(32));
	expect_static_false(nequal1);
	expect_static_true(nequal2);
	assert(nequal3->node()->operation() == jive::bits::ne_op(32));

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitnotequal", types_bitstring_comparison_test_bitnotequal);

static int types_bitstring_comparison_test_bitsgreater(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph->root_region, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph->root_region, 32, "s1");
	jive::output * c0 = jive_bitconstant_signed(graph->root_region, 32, 4);
	jive::output * c1 = jive_bitconstant_signed(graph->root_region, 32, 5);
	jive::output * c2 = jive_bitconstant_signed(graph->root_region, 32, 0x7fffffffL);
	jive::output * c3 = jive_bitconstant_signed(graph->root_region, 32, (-0x7fffffffL-1));

	jive::output * sgreater0 = jive_bitsgreater(s0, s1);
	jive::output * sgreater1 = jive_bitsgreater(c0, c1);
	jive::output * sgreater2 = jive_bitsgreater(c1, c0);
	jive::output * sgreater3 = jive_bitsgreater(s0, c2);
	jive::output * sgreater4 = jive_bitsgreater(c3, s1);

	jive_graph_export(graph, sgreater0);
	jive_graph_export(graph, sgreater1);
	jive_graph_export(graph, sgreater2);
	jive_graph_export(graph, sgreater3);
	jive_graph_export(graph, sgreater4);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(sgreater0->node()->operation() == jive::bits::sgt_op(32));
	expect_static_false(sgreater1);
	expect_static_true(sgreater2);
	expect_static_false(sgreater3);
	expect_static_false(sgreater4);

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitsgreater", types_bitstring_comparison_test_bitsgreater);

static int types_bitstring_comparison_test_bitsgreatereq(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph->root_region, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph->root_region, 32, "s1");
	jive::output * c0 = jive_bitconstant_signed(graph->root_region, 32, 4);
	jive::output * c1 = jive_bitconstant_signed(graph->root_region, 32, 5);
	jive::output * c2 = jive_bitconstant_signed(graph->root_region, 32, 0x7fffffffL);
	jive::output * c3 = jive_bitconstant_signed(graph->root_region, 32, (-0x7fffffffL-1));

	jive::output * sgreatereq0 = jive_bitsgreatereq(s0, s1);
	jive::output * sgreatereq1 = jive_bitsgreatereq(c0, c1);
	jive::output * sgreatereq2 = jive_bitsgreatereq(c1, c0);
	jive::output * sgreatereq3 = jive_bitsgreatereq(c0, c0);
	jive::output * sgreatereq4 = jive_bitsgreatereq(c2, s0);
	jive::output * sgreatereq5 = jive_bitsgreatereq(s1, c3);

	jive_graph_export(graph, sgreatereq0);
	jive_graph_export(graph, sgreatereq1);
	jive_graph_export(graph, sgreatereq2);
	jive_graph_export(graph, sgreatereq3);
	jive_graph_export(graph, sgreatereq4);
	jive_graph_export(graph, sgreatereq5);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(sgreatereq0->node()->operation() == jive::bits::sge_op(32));
	expect_static_false(sgreatereq1);
	expect_static_true(sgreatereq2);
	expect_static_true(sgreatereq3);
	expect_static_true(sgreatereq4);
	expect_static_true(sgreatereq5);

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitsgreatereq", types_bitstring_comparison_test_bitsgreatereq);

static int types_bitstring_comparison_test_bitsless(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph->root_region, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph->root_region, 32, "s1");
	jive::output * c0 = jive_bitconstant_signed(graph->root_region, 32, 4);
	jive::output * c1 = jive_bitconstant_signed(graph->root_region, 32, 5);
	jive::output * c2 = jive_bitconstant_signed(graph->root_region, 32, 0x7fffffffL);
	jive::output * c3 = jive_bitconstant_signed(graph->root_region, 32, (-0x7fffffffL-1));

	jive::output * sless0 = jive_bitsless(s0, s1);
	jive::output * sless1 = jive_bitsless(c0, c1);
	jive::output * sless2 = jive_bitsless(c1, c0);
	jive::output * sless3 = jive_bitsless(c2, s0);
	jive::output * sless4 = jive_bitsless(s1, c3);

	jive_graph_export(graph, sless0);
	jive_graph_export(graph, sless1);
	jive_graph_export(graph, sless2);
	jive_graph_export(graph, sless3);
	jive_graph_export(graph, sless4);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(sless0->node()->operation() == jive::bits::slt_op(32));
	expect_static_true(sless1);
	expect_static_false(sless2);
	expect_static_false(sless3);
	expect_static_false(sless4);

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitsless", types_bitstring_comparison_test_bitsless);

static int types_bitstring_comparison_test_bitslesseq(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph->root_region, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph->root_region, 32, "s1");
	jive::output * c0 = jive_bitconstant_signed(graph->root_region, 32, 4);
	jive::output * c1 = jive_bitconstant_signed(graph->root_region, 32, 5);
	jive::output * c2 = jive_bitconstant_signed(graph->root_region, 32, 0x7fffffffL);
	jive::output * c3 = jive_bitconstant_signed(graph->root_region, 32, (-0x7fffffffL-1));

	jive::output * slesseq0 = jive_bitslesseq(s0, s1);
	jive::output * slesseq1 = jive_bitslesseq(c0, c1);
	jive::output * slesseq2 = jive_bitslesseq(c0, c0);
	jive::output * slesseq3 = jive_bitslesseq(c1, c0);
	jive::output * slesseq4 = jive_bitslesseq(s0, c2);
	jive::output * slesseq5 = jive_bitslesseq(c3, s1);

	jive_graph_export(graph, slesseq0);
	jive_graph_export(graph, slesseq1);
	jive_graph_export(graph, slesseq2);
	jive_graph_export(graph, slesseq3);
	jive_graph_export(graph, slesseq4);
	jive_graph_export(graph, slesseq5);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(slesseq0->node()->operation() == jive::bits::sle_op(32));
	expect_static_true(slesseq1);
	expect_static_true(slesseq2);
	expect_static_false(slesseq3);
	expect_static_true(slesseq4);
	expect_static_true(slesseq5);

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitslesseq", types_bitstring_comparison_test_bitslesseq);

static int types_bitstring_comparison_test_bitugreater(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph->root_region, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph->root_region, 32, "s1");
	jive::output * c0 = jive_bitconstant_unsigned(graph->root_region, 32, 4);
	jive::output * c1 = jive_bitconstant_unsigned(graph->root_region, 32, 5);
	jive::output * c2 = jive_bitconstant_unsigned(graph->root_region, 32, (0xffffffffUL));
	jive::output * c3 = jive_bitconstant_unsigned(graph->root_region, 32, 0);

	jive::output * ugreater0 = jive_bitugreater(s0, s1);
	jive::output * ugreater1 = jive_bitugreater(c0, c1);
	jive::output * ugreater2 = jive_bitugreater(c1, c0);
	jive::output * ugreater3 = jive_bitugreater(s0, c2);
	jive::output * ugreater4 = jive_bitugreater(c3, s1);

	jive_graph_export(graph, ugreater0);
	jive_graph_export(graph, ugreater1);
	jive_graph_export(graph, ugreater2);
	jive_graph_export(graph, ugreater3);
	jive_graph_export(graph, ugreater4);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(ugreater0->node()->operation() == jive::bits::ugt_op(32));
	expect_static_false(ugreater1);
	expect_static_true(ugreater2);
	expect_static_false(ugreater3);
	expect_static_false(ugreater4);

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitugreater", types_bitstring_comparison_test_bitugreater);

static int types_bitstring_comparison_test_bitugreatereq(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph->root_region, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph->root_region, 32, "s1");
	jive::output * c0 = jive_bitconstant_signed(graph->root_region, 32, 4);
	jive::output * c1 = jive_bitconstant_signed(graph->root_region, 32, 5);
	jive::output * c2 = jive_bitconstant_signed(graph->root_region, 32, (0xffffffffUL));
	jive::output * c3 = jive_bitconstant_signed(graph->root_region, 32, 0);

	jive::output * ugreatereq0 = jive_bitugreatereq(s0, s1);
	jive::output * ugreatereq1 = jive_bitugreatereq(c0, c1);
	jive::output * ugreatereq2 = jive_bitugreatereq(c1, c0);
	jive::output * ugreatereq3 = jive_bitugreatereq(c0, c0);
	jive::output * ugreatereq4 = jive_bitugreatereq(c2, s0);
	jive::output * ugreatereq5 = jive_bitugreatereq(s1, c3);

	jive_graph_export(graph, ugreatereq0);
	jive_graph_export(graph, ugreatereq1);
	jive_graph_export(graph, ugreatereq2);
	jive_graph_export(graph, ugreatereq3);
	jive_graph_export(graph, ugreatereq4);
	jive_graph_export(graph, ugreatereq5);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(ugreatereq0->node()->operation() == jive::bits::uge_op(32));
	expect_static_false(ugreatereq1);
	expect_static_true(ugreatereq2);
	expect_static_true(ugreatereq3);
	expect_static_true(ugreatereq4);
	expect_static_true(ugreatereq5);

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitugreatereq", types_bitstring_comparison_test_bitugreatereq);

static int types_bitstring_comparison_test_bituless(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph->root_region, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph->root_region, 32, "s1");
	jive::output * c0 = jive_bitconstant_signed(graph->root_region, 32, 4);
	jive::output * c1 = jive_bitconstant_signed(graph->root_region, 32, 5);
	jive::output * c2 = jive_bitconstant_signed(graph->root_region, 32, (0xffffffffUL));
	jive::output * c3 = jive_bitconstant_signed(graph->root_region, 32, 0);

	jive::output * uless0 = jive_bituless(s0, s1);
	jive::output * uless1 = jive_bituless(c0, c1);
	jive::output * uless2 = jive_bituless(c1, c0);
	jive::output * uless3 = jive_bituless(c2, s0);
	jive::output * uless4 = jive_bituless(s1, c3);

	jive_graph_export(graph, uless0);
	jive_graph_export(graph, uless1);
	jive_graph_export(graph, uless2);
	jive_graph_export(graph, uless3);
	jive_graph_export(graph, uless4);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(uless0->node()->operation() == jive::bits::ult_op(32));
	expect_static_true(uless1);
	expect_static_false(uless2);
	expect_static_false(uless3);
	expect_static_false(uless4);

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bituless", types_bitstring_comparison_test_bituless);

static int types_bitstring_comparison_test_bitulesseq(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph->root_region, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph->root_region, 32, "s1");
	jive::output * c0 = jive_bitconstant_signed(graph->root_region, 32, 4);
	jive::output * c1 = jive_bitconstant_signed(graph->root_region, 32, 5);
	jive::output * c2 = jive_bitconstant_signed(graph->root_region, 32, (0xffffffffUL));
	jive::output * c3 = jive_bitconstant_signed(graph->root_region, 32, 0);

	jive::output * ulesseq0 = jive_bitulesseq(s0, s1);
	jive::output * ulesseq1 = jive_bitulesseq(c0, c1);
	jive::output * ulesseq2 = jive_bitulesseq(c0, c0);
	jive::output * ulesseq3 = jive_bitulesseq(c1, c0);
	jive::output * ulesseq4 = jive_bitulesseq(s0, c2);
	jive::output * ulesseq5 = jive_bitulesseq(c3, s1);

	jive_graph_export(graph, ulesseq0);
	jive_graph_export(graph, ulesseq1);
	jive_graph_export(graph, ulesseq2);
	jive_graph_export(graph, ulesseq3);
	jive_graph_export(graph, ulesseq4);
	jive_graph_export(graph, ulesseq5);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(ulesseq0->node()->operation() == jive::bits::ule_op(32));
	expect_static_true(ulesseq1);
	expect_static_true(ulesseq2);
	expect_static_false(ulesseq3);
	expect_static_true(ulesseq4);
	expect_static_true(ulesseq5);

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitulesseq", types_bitstring_comparison_test_bitulesseq);

static int types_bitstring_test_arithmetic(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * c0 = jive_bitconstant(graph->root_region, 4, "1100");
	jive::output * c1 = jive_bitconstant(graph->root_region, 4, "0001");

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

	jive_view(graph, stdout);

	jive_graph_destroy(graph);

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
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * b1 = jive_bitconstant(graph->root_region, 8, "00110011");
	jive::output * b2 = jive_bitconstant_unsigned(graph->root_region, 8, 204);
	jive::output * b3 = jive_bitconstant_signed(graph->root_region, 8, 204);
	jive::output * b4 = jive_bitconstant(graph->root_region, 9, "001100110");
	
	assert(b1->node()->operation() == jive::bits::uint_constant_op(8, 204));
	assert(b1->node()->operation() == jive::bits::int_constant_op(8, -52));

	assert(b1->node() == b2->node());
	assert(b1->node() == b3->node());
	
	assert(b1->node()->operation() == jive::bits::uint_constant_op(8, 204));
	assert(b1->node()->operation() == jive::bits::int_constant_op(8, -52));

	assert(b4->node()->operation() == jive::bits::uint_constant_op(9, 204));
	assert(b4->node()->operation() == jive::bits::int_constant_op(9, 204));

	jive::output * plus_one_128 = jive_bitconstant(graph->root_region, 128, ONE_64 ZERO_64);
	assert(plus_one_128->node()->operation() == jive::bits::uint_constant_op(128, 1));
	assert(plus_one_128->node()->operation() == jive::bits::int_constant_op(128, 1));

	jive::output * minus_one_128 = jive_bitconstant(graph->root_region, 128, MONE_64 MONE_64);
	assert(minus_one_128->node()->operation() == jive::bits::int_constant_op(128, -1));

	jive_view(graph, stdout);
	jive_graph_destroy(graph);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/test-constant", types_bitstring_test_constant);

static int types_bitstring_test_normalize(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::bits::type bits32(32);
	const char * tmparray0[] = {"arg"};
	const jive::base::type * tmparray11[] = {&bits32};
	jive_lambda * lambda = jive_lambda_begin(graph->root_region, 1, tmparray11, tmparray0);

	jive::output * c0 = jive_bitconstant_unsigned(lambda->region, 32, 3);
	jive::output * c1 = jive_bitconstant_unsigned(lambda->region, 32, 4);
	
	jive::node_normal_form * sum_nf = jive_graph_get_nodeclass_form(
		graph, typeid(jive::bits::add_op));
	assert(sum_nf);
	sum_nf->set_mutable(false);
	jive::output * tmparray1[] = {lambda->arguments[0], c0};

	jive::output * sum0 = jive_bitsum(2, tmparray1);
	assert(sum0->node()->operation() == jive::bits::add_op(32));
	assert(sum0->node()->noperands() == 2);
	jive::output * tmparray2[] = {sum0, c1};
	
	jive::output * sum1 = jive_bitsum(2, tmparray2);
	assert(sum1->node()->operation() == jive::bits::add_op(32));
	assert(sum1->node()->noperands() == 2);

	jive_node * lambda_node = jive_lambda_end(lambda, 1, tmparray11, &sum1)->node();
	jive::input * retval = lambda_node->producer(0)->input(1);
	jive::output * arg = lambda_node->producer(0)->producer(0)->outputs[1];
	jive_graph_export(graph, lambda_node->outputs[0]);
	
	sum_nf->set_mutable(true);
	jive_graph_normalize(graph);
	jive_graph_prune(graph);
	
	jive::output * expected_sum = retval->origin();
	assert(expected_sum->node()->operation() == jive::bits::add_op(32));
	assert(expected_sum->node()->noperands() == 2);
	jive::output * op1 = expected_sum->node()->input(0)->origin();
	jive::output * op2 = expected_sum->node()->input(1)->origin();
	if (!dynamic_cast<const jive::bits::constant_op *>(&op1->node()->operation())) {
		jive::output * tmp = op1; op1 = op2; op2 = tmp;
	}
	assert(op1->node()->operation() == jive::bits::int_constant_op(32, 3 + 4));
	assert(op2 == arg);

	jive_view(graph, stdout);

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/test-normalize", types_bitstring_test_normalize);

static void
assert_constant(jive::output * bitstr, size_t nbits, const char bits[])
{
	const jive::bits::constant_op & op =
		dynamic_cast<const jive::bits::constant_op &>(bitstr->node()->operation());
	
	assert(op.value() == jive::bits::value_repr(std::string(bits, nbits).c_str()));
}

static int types_bitstring_test_reduction(void)
{
	setlocale(LC_ALL, "");
	
	jive_graph * graph = jive_graph_create();
	
	jive::output * a = jive_bitconstant(graph->root_region, 4, "1100");
	jive::output * b = jive_bitconstant(graph->root_region, 4, "1010");
	
	jive::output * ops[] = {a, b};
	
	assert_constant(jive_bitand(2, ops), 4, "1000");
	assert_constant(jive_bitor(2, ops), 4, "1110");
	assert_constant(jive_bitxor(2, ops), 4, "0110");
	assert_constant(jive_bitsum(2, ops), 4, "0001");
	assert_constant(jive_bitmultiply(2, ops), 4, "1111");
	assert_constant(jive_bitconcat(2, ops), 8, "11001010");
	assert_constant(jive_bitnegate(a), 4, "1011");
	assert_constant(jive_bitnegate(b), 4, "1101");
	
	jive_graph_prune(graph);
	
	jive::output * x = jive_bitsymbolicconstant(graph->root_region, 16, "x");
	jive::output * y = jive_bitsymbolicconstant(graph->root_region, 16, "y");
	
	{
		jive::output *  tmparray0[] = {x, y};
		jive::output * concat = jive_bitconcat(2, tmparray0);
		jive::output * slice = jive_bitslice(concat, 8, 24);
		jive_node * node = ((jive::output *) slice)->node();
		assert(dynamic_cast<const jive::bits::concat_op *>(&node->operation()));
		assert(node->ninputs == 2);
		assert(dynamic_cast<const jive::bits::slice_op *>(&node->producer(0)->operation()));
		assert(dynamic_cast<const jive::bits::slice_op *>(&node->producer(1)->operation()));
		
		const jive::bits::slice_op * attrs;
		attrs = dynamic_cast<const jive::bits::slice_op*>(&node->producer(0)->operation());
		assert( (attrs->low() == 8) && (attrs->high() == 16) );
		attrs = dynamic_cast<const jive::bits::slice_op*>(&node->producer(1)->operation());
		assert( (attrs->low() == 0) && (attrs->high() == 8) );
		
		assert(node->producer(0)->input(0)->origin() == x);
		assert(node->producer(1)->input(0)->origin() == y);
	}
	
	{
		jive::output * slice1 = jive_bitslice(x, 0, 8);
		jive::output * slice2 = jive_bitslice(x, 8, 16);
		jive::output * tmparray1[] = {slice1, slice2};
		jive::output * concat = jive_bitconcat(2, tmparray1);
		assert(concat == x);
	}
	
	jive_graph_destroy(graph);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/test-reduction", types_bitstring_test_reduction);

static int types_bitstring_test_slice_concat(void)
{
	setlocale(LC_ALL, "");
	
	jive_graph * graph = jive_graph_create();
	
	jive::output * base_const1 = jive_bitconstant(graph->root_region, 8, "00110111");
	jive::output * base_const2 = jive_bitconstant(graph->root_region, 8, "11001000");
	
	jive::output * base_x = jive_bitsymbolicconstant(graph->root_region, 8, "x");
	jive::output * base_y = jive_bitsymbolicconstant(graph->root_region, 8, "y");
	jive::output * base_z = jive_bitsymbolicconstant(graph->root_region, 8, "z");
	
	{
		/* slice of constant */
		jive::output * a = jive_bitslice(base_const1, 2, 6);
		
		const jive::bits::constant_op & op =
			dynamic_cast<const jive::bits::constant_op &>(a->node()->operation());
		assert(op.value() == jive::bits::value_repr("1101"));
	}
	
	{
		/* slice of slice */
		jive::output * a = jive_bitslice(base_x, 2, 6);
		jive::output * b = jive_bitslice(a, 1, 3);

		assert(dynamic_cast<const jive::bits::slice_op *>(&b->node()->operation()));
		const jive::bits::slice_op * attrs;
		attrs = dynamic_cast<const jive::bits::slice_op *>(&b->node()->operation());
		assert(attrs->low() == 3 && attrs->high() == 5);
	}
	
	{
		/* slice of full node */
		jive::output * a = jive_bitslice(base_x, 0, 8);
		
		assert(a == base_x);
	}
	
	{
		/* slice of concat */
		jive::output * list1[] = {base_x, base_y};
		jive::output * a = jive_bitconcat(2, list1);
		jive::output * b = jive_bitslice(a, 0, 8);
		
		assert(static_cast<const jive::bits::type*>(&b->type())->nbits() == 8);
		
		assert(b == base_x);
	}
	
	{
		/* concat flattening */
		jive::output * list1[] = {base_x, base_y};
		jive::output * a = jive_bitconcat(2, list1);
		
		jive::output * list2[] = {a, base_z};
		jive::output * b = jive_bitconcat(2, list2);
		
		assert(dynamic_cast<const jive::bits::concat_op *>(&b->node()->operation()));
		assert(b->node()->ninputs == 3);
		assert(b->node()->input(0)->origin() == base_x);
		assert(b->node()->input(1)->origin() == base_y);
		assert(b->node()->input(2)->origin() == base_z);
	}
	
	{
		/* concat of single node */
		jive::output * a = jive_bitconcat(1, &base_x);
		
		assert(a==base_x);
	}
	
	{
		/* concat of slices */
		jive::output * a = jive_bitslice(base_x, 0, 4);
		jive::output * b = jive_bitslice(base_x, 4, 8);
		jive::output * list1[] = {a, b};
		jive::output * c = jive_bitconcat(2, list1);
		
		assert(c==base_x);
	}
	
	{
		/* concat of constants */
		jive::output * list1[] = {base_const1, base_const2};
		jive::output * a = jive_bitconcat(2, list1);
		
		const jive::bits::constant_op & op =
			dynamic_cast<const jive::bits::constant_op &>(a->node()->operation());
		assert(op.value() == jive::bits::value_repr("0011011111001000"));
	}
	
	{
		/* CSE */
		jive::output * a = jive_bitsymbolicconstant(graph->root_region, 8, "x");
		assert(a == base_x);
		
		jive::output * b = jive_bitconstant(graph->root_region, 8, "00110111");
		assert(b == base_const1);
		
		jive::output * c = jive_bitslice(base_x, 2, 6);
		jive::output * d = jive_bitslice(base_x, 2, 6);
		assert(c == d);
		
		jive::output * list1[] = {base_x, base_y};
		jive::output * e = jive_bitconcat(2, list1);
		jive::output * f = jive_bitconcat(2, list1);
		assert(e == f);
	}
	
	//jive_graph_view(graph);

	jive_graph_destroy(graph);
	
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
