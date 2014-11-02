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

#include <jive/context.h>
#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/node-private.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph, 32, "s1");
	jive::output * c0 = jive_bitconstant_signed(graph, 32, 4);
	jive::output * c1 = jive_bitconstant_signed(graph, 32, 5);
	jive::output * c2 = jive_bitconstant_signed(graph, 32, (0xffffffffUL));
	jive::output * c3 = jive_bitconstant_signed(graph, 32, 0);

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
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitulesseq", test_main);
