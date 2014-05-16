/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <locale.h>
#include <assert.h>

#include <jive/view.h>
#include <jive/view.h>
#include <jive/types/bitstring.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	jive_output * s0 = jive_bitsymbolicconstant(graph, 32, "s0");
	jive_output * s1 = jive_bitsymbolicconstant(graph, 32, "s1");

	jive_output * shiproduct = jive_bitshiproduct(s0, s1);

	jive_graph_export(graph, shiproduct);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(jive_node_isinstance(shiproduct->node, &JIVE_BITSHIPRODUCT_NODE));

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitshiproduct", test_main);
