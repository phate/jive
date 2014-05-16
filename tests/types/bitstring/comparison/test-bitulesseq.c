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
	jive_output * c2 = jive_bitconstant_signed(graph, 32, (0xffffffffUL));
	jive_output * c3 = jive_bitconstant_signed(graph, 32, 0);

	jive_output * ulesseq0 = jive_bitulesseq(s0, s1);
	jive_output * ulesseq1 = jive_bitulesseq(c0, c1);
	jive_output * ulesseq2 = jive_bitulesseq(c0, c0);
	jive_output * ulesseq3 = jive_bitulesseq(c1, c0);
	jive_output * ulesseq4 = jive_bitulesseq(s0, c2);
	jive_output * ulesseq5 = jive_bitulesseq(c3, s1);

	jive_graph_export(graph, ulesseq0);
	jive_graph_export(graph, ulesseq1);
	jive_graph_export(graph, ulesseq2);
	jive_graph_export(graph, ulesseq3);
	jive_graph_export(graph, ulesseq4);
	jive_graph_export(graph, ulesseq5);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(jive_node_isinstance(ulesseq0->node, &JIVE_BITULESSEQ_NODE));
	assert(jive_node_isinstance(ulesseq1->node, &JIVE_CONTROL_TRUE_NODE));
	assert(jive_node_isinstance(ulesseq2->node, &JIVE_CONTROL_TRUE_NODE));
	assert(jive_node_isinstance(ulesseq3->node, &JIVE_CONTROL_FALSE_NODE));
	assert(jive_node_isinstance(ulesseq4->node, &JIVE_CONTROL_TRUE_NODE));
	assert(jive_node_isinstance(ulesseq5->node, &JIVE_CONTROL_TRUE_NODE));

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitulesseq", test_main);
