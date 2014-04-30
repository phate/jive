/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/addresstype.h>
#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	jive_address_type addrtype;
	jive_bitstring_type bits32(32);
	jive_bitstring_type bits64(64);
	const jive_type * tmparray0[] = {&addrtype, &bits32, &bits64};
	jive_node * top = jive_node_create(graph->root_region,
		0, NULL, NULL,
		3, tmparray0);

	jive_output * b0 = jive_address_to_bitstring_create(top->outputs[0], 32,
		jive_output_get_type(top->outputs[0]));
	jive_output * a0 = jive_bitstring_to_address_create(b0, 32, &addrtype);

	jive_output * a1 = jive_bitstring_to_address_create(top->outputs[1], 32, &addrtype);
	jive_output * b1 = jive_address_to_bitstring_create(a1, 32, &addrtype);
	const jive_type* tmparray1[] = {&addrtype, &bits32};
	jive_output * tmparray2[] = {a0, b1};

	jive_node * bottom = jive_node_create(graph->root_region,
		2, tmparray1, tmparray2,
		0, NULL);

	assert(bottom->inputs[0]->origin == top->outputs[0]);
	assert(bottom->inputs[1]->origin == top->outputs[1]);

	jive_output * b2 = jive_bitstring_to_address_create(top->outputs[1], 32, &addrtype);
	jive_output * b3 = jive_bitstring_to_address_create(top->outputs[1], 32, &addrtype);
	jive_output * a2 = jive_address_to_bitstring_create(top->outputs[0], 32,
		jive_output_get_type(top->outputs[0]));
	jive_output * a3 = jive_address_to_bitstring_create(top->outputs[0], 32,
		jive_output_get_type(top->outputs[0]));
	
	assert(jive_node_match_attrs(a2->node, jive_node_get_attrs(a3->node)));
	assert(jive_node_match_attrs(b2->node, jive_node_get_attrs(b3->node)));

	jive_output * b4 = jive_bitstring_to_address_create(top->outputs[2], 64, &addrtype);
	jive_output * a4 = jive_address_to_bitstring_create(top->outputs[0], 64, &addrtype);

	assert(!jive_node_match_attrs(a2->node, jive_node_get_attrs(a4->node)));
	assert(!jive_node_match_attrs(b2->node, jive_node_get_attrs(b4->node)));
	
	jive_view(graph, stderr);

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/address-transform/test-address-transform-nodes", test_main);
