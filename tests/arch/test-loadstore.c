/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/arch/addresstype.h>
#include <jive/arch/address-transform.h>
#include <jive/arch/load.h>
#include <jive/arch/store.h>
#include <jive/arch/memorytype.h>
#include <jive/vsdg/node-private.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");
	
	jive_context * context = jive_context_create();
	
	jive_graph * graph = jive_graph_create(context);
	
	JIVE_DECLARE_ADDRESS_TYPE(addrtype);
	JIVE_DECLARE_BITSTRING_TYPE(valuetype, 32);
	JIVE_DECLARE_MEMORY_TYPE(memtype);
	
	jive_node * top = jive_node_create(graph->root_region,
		0, NULL, NULL,
		2, (const jive_type *[]) {addrtype, memtype});
	
	jive_output * address = top->outputs[0];
	jive_output * memstate = top->outputs[1];
	
	jive_node * load = jive_load_by_address_node_create(graph->root_region,
		address, jive_value_type_cast(valuetype),
		1, (jive_output *[]) {memstate});
	
	jive_output * value = load->outputs[0];
	
	jive_node * store = jive_store_by_address_node_create(graph->root_region,
		address, jive_value_type_cast(valuetype), value,
		1, (jive_output *[]) {memstate});
	
	memstate = store->outputs[0];
	
	jive_node * bottom = jive_node_create(graph->root_region,
		1, (const jive_type *[]) {memtype}, (jive_output *[]){memstate},
		0, NULL);
	jive_node_reserve(bottom);
	
	jive_view(graph, stdout);

	jive_load_node_address_transform(jive_load_node_cast(load), 64);
	jive_store_node_address_transform(jive_store_node_cast(store), 64);

	jive_graph_prune(graph);
	jive_view(graph, stdout);
	
	jive_context * context2 = jive_context_create();
	jive_graph * graph2 = jive_graph_copy(graph, context2);
	
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);
	
	jive_view(graph2, stdout);
	
	jive_graph_destroy(graph2);
	assert(jive_context_is_empty(context2));
	jive_context_destroy(context2);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-loadstore", test_main);
