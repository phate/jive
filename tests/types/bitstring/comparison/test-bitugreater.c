/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include "bitcmp-test-helpers.h"

#include <assert.h>
#include <locale.h>
#include <stdint.h>

#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/node-private.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph, 32, "s1");
	jive::output * c0 = jive_bitconstant_unsigned(graph, 32, 4);
	jive::output * c1 = jive_bitconstant_unsigned(graph, 32, 5);
	jive::output * c2 = jive_bitconstant_unsigned(graph, 32, (0xffffffffUL));
	jive::output * c3 = jive_bitconstant_unsigned(graph, 32, 0);

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


JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitugreater", test_main);
