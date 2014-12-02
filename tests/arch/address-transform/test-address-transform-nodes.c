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

#include "testnodes.h"

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::addr::type addrtype;
	jive::bits::type bits32(32);
	jive::bits::type bits64(64);
	const jive::base::type * tmparray0[] = {&addrtype, &bits32, &bits64};
	jive_node * top = jive_test_node_create(graph->root_region,
		0, NULL, NULL,
		3, tmparray0);

	jive::output * b0 = jive_address_to_bitstring_create(top->outputs[0], 32,
		&top->outputs[0]->type());
	jive::output * a0 = jive_bitstring_to_address_create(b0, 32, &addrtype);

	jive::output * a1 = jive_bitstring_to_address_create(top->outputs[1], 32, &addrtype);
	jive::output * b1 = jive_address_to_bitstring_create(a1, 32, &addrtype);
	const jive::base::type* tmparray1[] = {&addrtype, &bits32};
	jive::output * tmparray2[] = {a0, b1};

	jive_node * bottom = jive_test_node_create(graph->root_region,
		2, tmparray1, tmparray2,
		0, NULL);

	assert(bottom->inputs[0]->origin() == top->outputs[0]);
	assert(bottom->inputs[1]->origin() == top->outputs[1]);

	jive::output * b2 = jive_bitstring_to_address_create(top->outputs[1], 32, &addrtype);
	jive::output * b3 = jive_bitstring_to_address_create(top->outputs[1], 32, &addrtype);
	jive::output * a2 = jive_address_to_bitstring_create(top->outputs[0], 32,
		&top->outputs[0]->type());
	jive::output * a3 = jive_address_to_bitstring_create(top->outputs[0], 32,
		&top->outputs[0]->type());
	
	assert(jive_node_match_attrs(a2->node(), jive_node_get_attrs(a3->node())));
	assert(jive_node_match_attrs(b2->node(), jive_node_get_attrs(b3->node())));

	jive::output * b4 = jive_bitstring_to_address_create(top->outputs[2], 64, &addrtype);
	jive::output * a4 = jive_address_to_bitstring_create(top->outputs[0], 64, &addrtype);

	assert(!jive_node_match_attrs(a2->node(), jive_node_get_attrs(a4->node())));
	assert(!jive_node_match_attrs(b2->node(), jive_node_get_attrs(b4->node())));
	
	jive_view(graph, stderr);

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/address-transform/test-address-transform-nodes", test_main);
