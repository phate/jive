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

	jive::bits::type bits32(32);
	const jive::base::type * tmparray0[] = {&bits32, &bits32};

	jive::output * s0 = jive_bitsymbolicconstant(graph, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph, 32, "s1");
	jive::output * c0 = jive_bitconstant_signed(graph, 32, 4);
	jive::output * c1 = jive_bitconstant_signed(graph, 32, 5);
	jive::output * c2 = jive_bitconstant_signed(graph, 32, 0x7fffffffL);
	jive::output * c3 = jive_bitconstant_signed(graph, 32, (-0x7fffffffL-1));

	jive::output * sgreater0 = jive_bitsgreater(s0, s1);
	jive::output * sgreater1 = jive_bitsgreater(c0, c1);
	jive::output * sgreater2 = jive_bitsgreater(c1, c0);
	jive::output * sgreater3 = jive_bitsgreater(s0, c2);
	jive::output * sgreater4 = jive_bitsgreater(c3, s1);

	jive_graph_export(graph, sgreater0);
	jive_graph_export(graph, sgreater1);
	jive_graph_export(graph, sgreater2);
	jive_graph_export(graph, sgreater3);
	jive_graph_export(graph, sgreater4);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(jive_node_isinstance(sgreater0->node(), &JIVE_BITSGREATER_NODE));
	assert(jive_node_isinstance(sgreater1->node(), &JIVE_CONTROL_FALSE_NODE));
	assert(jive_node_isinstance(sgreater2->node(), &JIVE_CONTROL_TRUE_NODE));
	assert(jive_node_isinstance(sgreater3->node(), &JIVE_CONTROL_FALSE_NODE));
	assert(jive_node_isinstance(sgreater4->node(), &JIVE_CONTROL_FALSE_NODE));

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitsgreater", test_main);
