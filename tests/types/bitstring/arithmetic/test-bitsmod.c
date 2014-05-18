/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <locale.h>
#include <assert.h>

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

	jive_output * c0 = jive_bitconstant_signed(graph, 32, -7);
	jive_output * c1 = jive_bitconstant_signed(graph, 32, 3);

	jive_output * smod0 = jive_bitsmod(s0, s1);
	jive_output * smod1 = jive_bitsmod(c0, c1);

	jive_graph_export(graph, smod0);
	jive_graph_export(graph, smod1);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(jive_node_isinstance(smod0->node, &JIVE_BITSMOD_NODE));
	assert(jive_node_isinstance(smod1->node, &JIVE_BITCONSTANT_NODE));

	jive_bitconstant_node * bc1 = dynamic_cast<jive_bitconstant_node *>(smod1->node);
	assert(jive_bitconstant_equals_signed(bc1, -1));
	
	jive_graph_destroy(graph);
	jive_context_assert_clean(context);
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitsmod", test_main);
