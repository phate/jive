/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg.h>

#define ZERO_64 \
	"00000000" "00000000" "00000000" "00000000" \
	"00000000" "00000000" "00000000" "00000000"
#define ONE_64 \
	"10000000" "00000000" "00000000" "00000000" \
	"00000000" "00000000" "00000000" "00000000"
#define MONE_64 \
	"11111111" "11111111" "11111111" "11111111" \
	"11111111" "11111111" "11111111" "11111111"

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * b1 = jive_bitconstant(graph, 8, "00110011");
	jive::output * b2 = jive_bitconstant_unsigned(graph, 8, 204);
	jive::output * b3 = jive_bitconstant_signed(graph, 8, 204);
	jive::output * b4 = jive_bitconstant(graph, 9, "001100110");
	
	assert(b1->node()->operation() == jive::bits::uint_constant_op(8, 204));
	assert(b1->node()->operation() == jive::bits::int_constant_op(8, -52));

	assert(b1->node() == b2->node());
	assert(b1->node() == b3->node());
	
	assert(b1->node()->operation() == jive::bits::uint_constant_op(8, 204));
	assert(b1->node()->operation() == jive::bits::int_constant_op(8, -52));

	assert(b4->node()->operation() == jive::bits::uint_constant_op(9, 204));
	assert(b4->node()->operation() == jive::bits::int_constant_op(9, 204));

	jive::output * plus_one_128 = jive_bitconstant(graph, 128, ONE_64 ZERO_64);
	assert(plus_one_128->node()->operation() == jive::bits::uint_constant_op(128, 1));
	assert(plus_one_128->node()->operation() == jive::bits::int_constant_op(128, 1));

	jive::output * minus_one_128 = jive_bitconstant(graph, 128, MONE_64 MONE_64);
	assert(minus_one_128->node()->operation() != jive::bits::uint_constant_op(128, -1));
	assert(minus_one_128->node()->operation() == jive::bits::int_constant_op(128, -1));

	jive_view(graph, stdout);
	jive_graph_destroy(graph);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/test-constant", test_main);
