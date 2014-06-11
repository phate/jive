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

	jive::output * s0 = jive_bitsymbolicconstant(graph, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph, 32, "s1");
	jive::output * c0 = jive_bitconstant_signed(graph, 32, 4);
	jive::output * c1 = jive_bitconstant_signed(graph, 32, 5);
	jive::output * c2 = jive_bitconstant_signed(graph, 32, (0xffffffffUL));
	jive::output * c3 = jive_bitconstant_signed(graph, 32, 0);

	jive::output * ugreatereq0 = jive_bitugreatereq(s0, s1);
	jive::output * ugreatereq1 = jive_bitugreatereq(c0, c1);
	jive::output * ugreatereq2 = jive_bitugreatereq(c1, c0);
	jive::output * ugreatereq3 = jive_bitugreatereq(c0, c0);
	jive::output * ugreatereq4 = jive_bitugreatereq(c2, s0);
	jive::output * ugreatereq5 = jive_bitugreatereq(s1, c3);

	jive_graph_export(graph, ugreatereq0);
	jive_graph_export(graph, ugreatereq1);
	jive_graph_export(graph, ugreatereq2);
	jive_graph_export(graph, ugreatereq3);
	jive_graph_export(graph, ugreatereq4);
	jive_graph_export(graph, ugreatereq5);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(jive_node_isinstance(ugreatereq0->node(), &JIVE_BITUGREATEREQ_NODE));
	assert(jive_node_isinstance(ugreatereq1->node(), &JIVE_CONTROL_FALSE_NODE));
	assert(jive_node_isinstance(ugreatereq2->node(), &JIVE_CONTROL_TRUE_NODE));
	assert(jive_node_isinstance(ugreatereq3->node(), &JIVE_CONTROL_TRUE_NODE));
	assert(jive_node_isinstance(ugreatereq4->node(), &JIVE_CONTROL_TRUE_NODE));
	assert(jive_node_isinstance(ugreatereq5->node(), &JIVE_CONTROL_TRUE_NODE));

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitugreatereq", test_main);
