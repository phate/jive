/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <locale.h>
#include <assert.h>
#include <stdint.h>

#include <jive/vsdg.h>
#include <jive/view.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/node-private.h>
#include <jive/types/bitstring.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	jive_output * s0 = jive_bitsymbolicconstant(graph, 32, "s0");
	jive_output * s1 = jive_bitsymbolicconstant(graph, 32, "s1");
	jive_output * c0 = jive_bitconstant_signed(graph, 32, 4);
	jive_output * c1 = jive_bitconstant_signed(graph, 32, 5);
	jive_output * c2 = jive_bitconstant_signed(graph, 32, 0x7fffffffL);
	jive_output * c3 = jive_bitconstant_signed(graph, 32, (-0x7fffffffL-1));

	jive_output * sless0 = jive_bitsless(s0, s1);
	jive_output * sless1 = jive_bitsless(c0, c1);
	jive_output * sless2 = jive_bitsless(c1, c0);
	jive_output * sless3 = jive_bitsless(c2, s0);
	jive_output * sless4 = jive_bitsless(s1, c3);

	jive_graph_export(graph, sless0);
	jive_graph_export(graph, sless1);
	jive_graph_export(graph, sless2);
	jive_graph_export(graph, sless3);
	jive_graph_export(graph, sless4);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(jive_node_isinstance(sless0->node, &JIVE_BITSLESS_NODE));
	assert(jive_node_isinstance(sless1->node, &JIVE_CONTROL_TRUE_NODE));
	assert(jive_node_isinstance(sless2->node, &JIVE_CONTROL_FALSE_NODE));
	assert(jive_node_isinstance(sless3->node, &JIVE_CONTROL_FALSE_NODE));
	assert(jive_node_isinstance(sless4->node, &JIVE_CONTROL_FALSE_NODE));

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitsless", test_main);
