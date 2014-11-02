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
	jive::output * c2 = jive_bitconstant_signed(graph, 32, 0x7fffffffL);
	jive::output * c3 = jive_bitconstant_signed(graph, 32, (-0x7fffffffL-1));

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
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}


JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitslesseq", test_main);
