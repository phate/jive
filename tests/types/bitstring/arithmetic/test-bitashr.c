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
#include <jive/vsdg/node-private.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph, 32, "s1");

	jive::output * c0 = jive_bitconstant_unsigned(graph, 32, 16);
	jive::output * c1 = jive_bitconstant_signed(graph, 32, -16);
	jive::output * c2 = jive_bitconstant_unsigned(graph, 32, 2);
	jive::output * c3 = jive_bitconstant_unsigned(graph, 32, 32);

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

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitashr", test_main);
