/*
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/arch/load.h>
#include <jive/arch/store.h>
#include <jive/arch/memorytype.h>
#include <jive/arch/addresstype.h>
#include <jive/types/bitstring/type.h>
#include <jive/vsdg/node-private.h>

static int test_main()
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	JIVE_DECLARE_MEMORY_TYPE(memtype);
	JIVE_DECLARE_ADDRESS_TYPE(addrtype);
	JIVE_DECLARE_BITSTRING_TYPE(bits32, 32);
	jive_node * top = jive_node_create(graph->root_region,
		0, NULL, NULL,
		4, (const jive_type *[]){addrtype, addrtype, memtype, bits32});

	jive_output * load0 = jive_load_by_address_create(top->outputs[0],
		jive_value_type_cast(bits32), 1, &top->outputs[2]);

	jive_output * state;
	jive_store_by_address_create(top->outputs[1], jive_value_type_cast(bits32),
		top->outputs[3], 1, &top->outputs[2], &state);
	jive_output * load1 = jive_load_by_address_create(top->outputs[1],
		jive_value_type_cast(bits32), 1, &state);
	assert(load1 == top->outputs[3]);

	jive_node * bottom = jive_node_create(graph->root_region,
		2, (const jive_type *[]){bits32, bits32}, (jive_output *[]){load0, load1},
		0, NULL);
	jive_node_reserve(bottom);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);

	jive_view(graph, stderr);

	assert(bottom->inputs[1]->origin->node == top);

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-load", test_main);
