/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <locale.h>
#include <assert.h>

#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/arch/addresstype.h>
#include <jive/arch/address.h>
#include <jive/arch/address-transform.h>
#include <jive/arch/call.h>
#include <jive/arch/linker-symbol.h>
#include <jive/arch/load.h>
#include <jive/arch/store.h>
#include <jive/arch/memlayout-simple.h>
#include <jive/arch/memorytype.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/label.h>

#include "testnodes.h"

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	jive::addr::type addr;
	jive::mem::type mem;
	jive::bits::type bits64(64);
	const jive_type * tmparray0[] = {&bits64, &bits64, &mem};
	jive_node * top = jive_test_node_create(graph->root_region,
		0, NULL, NULL,
		3, tmparray0);

	jive_output * address0 = jive_bitstring_to_address_create(top->outputs[0], 64, &addr);
	jive_output * address1 = jive_bitstring_to_address_create(top->outputs[1], 64, &addr);
	const jive_value_type * tmparray1[] = {&addr, &addr};

	jive::rcd::declaration decl = {2, tmparray1};

	jive_output * memberof = jive_memberof(address0, &decl, 0);
	jive_output * containerof = jive_containerof(address1, &decl, 1);

	jive_linker_symbol write_symbol;
	jive_label_external write_label;
	jive_label_external_init(&write_label, context, "write", &write_symbol);
	jive_output * label = jive_label_to_address_create(graph, &write_label.base);
	jive_output * tmparray2[] = {memberof, containerof};
	const jive_type * tmparray3[] = {&addr, &addr};
	jive_node * call = jive_call_by_address_node_create(graph->root_region,
		label, NULL,
		2, tmparray2,
		2, tmparray3);

	jive_output * constant = jive_bitconstant_unsigned(graph, 64, 1);	
	jive_output * arraysub = jive_arraysubscript(call->outputs[0], &addr, constant);

	jive_output * arrayindex = jive_arrayindex(call->outputs[0], call->outputs[1], &addr, &bits64);
	
	jive_output * load = jive_load_by_address_create(arraysub, &addr, 1, &top->outputs[2]);
	jive_node * store = jive_store_by_address_node_create(graph->root_region, arraysub,
		&bits64, arrayindex, 1, &top->outputs[2]);

	jive_output * o_addr = jive_address_to_bitstring_create(load, 64, &load->type());
	const jive_type * tmparray4[] = {&bits64, &mem};
	jive_output * tmparray5[] = {o_addr, store->outputs[0]};
	
	jive_node * bottom = jive_test_node_create(graph->root_region,
		2, tmparray4, tmparray5,
		1, tmparray3);
	jive_graph_export(graph, bottom->outputs[0]);

	jive_view(graph, stdout);

	jive_memlayout_mapper_simple mapper;
	jive_memlayout_mapper_simple_init(&mapper, context, 64);

	jive_graph_address_transform(graph, &mapper.base.base);

	jive_graph_prune(graph);
	jive_view(graph, stdout);
	
	jive_graph_destroy(graph);
	jive_label_external_fini(&write_label);
	jive_memlayout_mapper_simple_fini(&mapper);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/address-transform/test-address-transform", test_main);
