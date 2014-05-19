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

	jive_output * c0 = jive_bitconstant_unsigned(graph, 32, 3);
	jive_output * c1 = jive_bitconstant_unsigned(graph, 32, 5);
	jive_output * tmparray1[] = {s0, s1};

	jive_output * and0 = jive_bitand(2, tmparray1);
	jive_output * tmparray2[] = {c0, c1};
	jive_output * and1 = jive_bitand(2, tmparray2);

	jive_graph_export(graph, and0);
	jive_graph_export(graph, and1);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(jive_node_isinstance(and0->node(), &JIVE_BITAND_NODE));
	assert(jive_node_isinstance(and1->node(), &JIVE_BITCONSTANT_NODE));

	jive_bitconstant_node * bc1 = dynamic_cast<jive_bitconstant_node *>(and1->node());
	assert(jive_bitconstant_equals_unsigned(bc1, 1));

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitand", test_main);
