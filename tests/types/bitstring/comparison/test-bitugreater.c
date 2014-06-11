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

	assert(jive_node_isinstance(ugreater0->node(), &JIVE_BITUGREATER_NODE));
	assert(jive_node_isinstance(ugreater1->node(), &JIVE_CONTROL_FALSE_NODE));
	assert(jive_node_isinstance(ugreater2->node(), &JIVE_CONTROL_TRUE_NODE));
	assert(jive_node_isinstance(ugreater3->node(), &JIVE_CONTROL_FALSE_NODE));
	assert(jive_node_isinstance(ugreater4->node(), &JIVE_CONTROL_FALSE_NODE));

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}


JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitugreater", test_main);
