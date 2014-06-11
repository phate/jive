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
	jive::output * c2 = jive_bitconstant_undefined(graph, 32);

	jive::output * nequal0 = jive_bitnotequal(s0, s1);
	jive::output * nequal1 = jive_bitnotequal(c0, c0);
	jive::output * nequal2 = jive_bitnotequal(c0, c1);
	jive::output * nequal3 = jive_bitnotequal(c0, c2);

	jive_graph_export(graph, nequal0);
	jive_graph_export(graph, nequal1);
	jive_graph_export(graph, nequal2);
	jive_graph_export(graph, nequal3);
	
	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(jive_node_isinstance(nequal0->node(), &JIVE_BITNOTEQUAL_NODE));
	assert(jive_node_isinstance(nequal1->node(), &JIVE_CONTROL_FALSE_NODE));
	assert(jive_node_isinstance(nequal2->node(), &JIVE_CONTROL_TRUE_NODE));
	assert(jive_node_isinstance(nequal3->node(), &JIVE_BITNOTEQUAL_NODE));

	jive_graph_destroy(graph);
	jive_context_assert_clean(context);
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitnotequal", test_main);
