/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
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

	JIVE_DECLARE_BITSTRING_TYPE(bits32, 32);
	const jive_type * tmparray0[] = {bits32, bits32};
	jive_node * top = jive_node_create(graph->root_region,
		0, NULL, NULL,
		2, tmparray0);

	jive_output * c0 = jive_bitconstant_signed(graph, 32, -7);
	jive_output * c1 = jive_bitconstant_signed(graph, 32, 3);

	jive_output * smod0 = jive_bitsmod(top->outputs[0], top->outputs[1]);
	jive_output * smod1 = jive_bitsmod(c0, c1);
const jive_type * tmparray1[] = {bits32, bits32};
jive_output * tmparray2[] = {smod0, smod1};

	jive_node * bottom = jive_node_create(graph->root_region,
		2, tmparray1, tmparray2, 1, &bits32);
	jive_graph_export(graph, bottom->outputs[0]);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(jive_node_isinstance(bottom->inputs[0]->origin->node, &JIVE_BITSMOD_NODE));
	assert(jive_node_isinstance(bottom->inputs[1]->origin->node, &JIVE_BITCONSTANT_NODE));

	jive_bitconstant_node * bc1 = jive_bitconstant_node_cast(bottom->inputs[1]->origin->node);
	assert(jive_bitconstant_equals_signed(bc1, -1));
	
	jive_graph_destroy(graph);
	jive_context_assert_clean(context);
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitsmod", test_main);
