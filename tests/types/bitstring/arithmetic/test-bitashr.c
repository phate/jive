/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <locale.h>
#include <assert.h>

#include <jive/vsdg.h>
#include <jive/view.h>
#include <jive/types/bitstring.h>
#include <jive/vsdg/node-private.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	jive_output * s0 = jive_bitsymbolicconstant(graph, 32, "s0");
	jive_output * s1 = jive_bitsymbolicconstant(graph, 32, "s1");

	jive_output * c0 = jive_bitconstant_unsigned(graph, 32, 16);
	jive_output * c1 = jive_bitconstant_signed(graph, 32, -16);
	jive_output * c2 = jive_bitconstant_unsigned(graph, 32, 2);
	jive_output * c3 = jive_bitconstant_unsigned(graph, 32, 32);

	jive_output * ashr0 = jive_bitashr(s0, s1);
	jive_output * ashr1 = jive_bitashr(c0, c2);
	jive_output * ashr2 = jive_bitashr(c0, c3);
	jive_output * ashr3 = jive_bitashr(c1, c2);
	jive_output * ashr4 = jive_bitashr(c1, c3);

	jive_graph_export(graph, ashr0);
	jive_graph_export(graph, ashr1);
	jive_graph_export(graph, ashr2);
	jive_graph_export(graph, ashr3);
	jive_graph_export(graph, ashr4);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(jive_node_isinstance(ashr0->node(), &JIVE_BITASHR_NODE));
	assert(jive_node_isinstance(ashr1->node(), &JIVE_BITCONSTANT_NODE));
	assert(jive_node_isinstance(ashr2->node(), &JIVE_BITCONSTANT_NODE));
	assert(jive_node_isinstance(ashr3->node(), &JIVE_BITCONSTANT_NODE));
	assert(jive_node_isinstance(ashr4->node(), &JIVE_BITCONSTANT_NODE));

	jive_bitconstant_node * bc1 = dynamic_cast<jive_bitconstant_node *>(ashr1->node());
	jive_bitconstant_node * bc2 = dynamic_cast<jive_bitconstant_node *>(ashr2->node());
	jive_bitconstant_node * bc3 = dynamic_cast<jive_bitconstant_node *>(ashr3->node());
	jive_bitconstant_node * bc4 = dynamic_cast<jive_bitconstant_node *>(ashr4->node());
	assert(jive_bitconstant_equals_unsigned(bc1, 4));
	assert(jive_bitconstant_equals_unsigned(bc2, 0));
	assert(jive_bitconstant_equals_signed(bc3, -4));
	assert(jive_bitconstant_equals_signed(bc4, -1));

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitashr", test_main);
