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

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	jive::output * s0 = jive_bitsymbolicconstant(graph, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph, 32, "s1");
	jive::output * c0 = jive_bitconstant_signed(graph, 32, 4);
	jive::output * c1 = jive_bitconstant_signed(graph, 32, 5);
	jive::output * c2 = jive_bitconstant_signed(graph, 32, (0xffffffffUL));
	jive::output * c3 = jive_bitconstant_signed(graph, 32, 0);

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
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}


JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bituless", test_main);
