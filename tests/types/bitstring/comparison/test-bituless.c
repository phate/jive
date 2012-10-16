/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <locale.h>
#include <assert.h>
#include <stdint.h>

#include <jive/vsdg.h>
#include <jive/view.h>
#include <jive/vsdg/node-private.h>
#include <jive/types/bitstring/constant.h>
#include <jive/types/bitstring/comparison/bituless.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	JIVE_DECLARE_BITSTRING_TYPE(bits32, 32);
	jive_node * top = jive_node_create(graph->root_region,
		0, NULL, NULL,
		2, (const jive_type *[]){bits32, bits32});
	jive_node_reserve(top);

	jive_output * c0 = jive_bitconstant_signed(graph, 32, 4);
	jive_output * c1 = jive_bitconstant_signed(graph, 32, 5);
	jive_output * c2 = jive_bitconstant_signed(graph, 32, UINT32_MAX);
	jive_output * c3 = jive_bitconstant_signed(graph, 32, 0);

	jive_output * uless0 = jive_bituless(top->outputs[0], top->outputs[1]);
	jive_output * uless1 = jive_bituless(c0, c1);
	jive_output * uless2 = jive_bituless(c1, c0);
	jive_output * uless3 = jive_bituless(c2, top->outputs[0]);
	jive_output * uless4 = jive_bituless(top->outputs[1], c3);

	JIVE_DECLARE_CONTROL_TYPE(ctype);
	jive_node * bottom = jive_node_create(graph->root_region,
		5, (const jive_type *[]){ctype, ctype, ctype, ctype, ctype},
		(jive_output *[]){uless0, uless1, uless2, uless3, uless4}, 0, NULL);
	jive_node_reserve(bottom);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(jive_node_isinstance(bottom->inputs[0]->origin->node, &JIVE_BITULESS_NODE));
	assert(jive_node_isinstance(bottom->inputs[1]->origin->node, &JIVE_CONTROL_TRUE_NODE));
	assert(jive_node_isinstance(bottom->inputs[2]->origin->node, &JIVE_CONTROL_FALSE_NODE));
	assert(jive_node_isinstance(bottom->inputs[3]->origin->node, &JIVE_CONTROL_FALSE_NODE));
	assert(jive_node_isinstance(bottom->inputs[4]->origin->node, &JIVE_CONTROL_FALSE_NODE));

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}


JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bituless", test_main);
