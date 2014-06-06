/*
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <locale.h>
#include <assert.h>

#include <jive/arch/addresstype.h>
#include <jive/arch/memorytype.h>
#include <jive/arch/store.h>
#include <jive/types/bitstring/type.h>
#include <jive/types/record.h>
#include <jive/types/union.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>

#include "testnodes.h"

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);
	
	static const jive::bits::type bits8(8);
	static const jive::bits::type bits16(16);
	static const jive::bits::type bits32(32);

	static const jive_value_type * decl_elems[] = {&bits8, &bits16, &bits32};
	static const jive_record_declaration rcddecl = {3, decl_elems};
	static jive_record_type rcdtype(&rcddecl);
	
	static const jive_union_declaration unndecl = {3, decl_elems};
	static jive_union_type unntype(&unndecl);

	static const jive_union_declaration empty_unndecl = {0, NULL};
	static jive_union_type empty_unntype(&empty_unndecl);

	jive::mem::type memtype;
	jive::addr::type addrtype;
	const jive_type * tmparray0[] = {&addrtype, &memtype, &bits8, &bits16, &bits32, &memtype};
	jive_node * top = jive_test_node_create(graph->root_region,
		0, NULL, NULL,
		6, tmparray0);

	jive_output * state0;
	jive_store_by_address_create(top->outputs[0], &bits32, top->outputs[4],
		1, &top->outputs[1], &state0);

	jive_output * state1[2];
	jive_output * tmparray1[] = {top->outputs[2],
		top->outputs[3], top->outputs[4]};
	jive_output * group = jive_group_create(&rcddecl, 3, tmparray1);
	jive_output * tmparray2[] = {top->outputs[1], top->outputs[5]};
	jive_store_by_address_create(top->outputs[0], &rcdtype, group,
		2, tmparray2, state1);

	jive_output * state2;
	jive_output * unify = jive_unify_create(&unndecl, 2, top->outputs[4]);
	jive_store_by_address_create(top->outputs[0], &unntype, unify,
		1, &top->outputs[1], &state2);

	jive_output * state3;
	jive_store_by_address_create(top->outputs[0], &bits32, top->outputs[4],
		1, &top->outputs[1], &state3);
	jive_store_by_address_create(top->outputs[0], &bits32, top->outputs[4],
		1, &state3, &state3);

	jive_output * state4;
	unify = jive_empty_unify_create(graph, &empty_unndecl);
	jive_store_by_address_create(top->outputs[0], &empty_unntype, unify,
		1, &top->outputs[1], &state4);
	const jive_type * tmparray3[] = {&memtype, &memtype, &memtype, &memtype, &memtype, &memtype};
	jive_output * tmparray4[] = {state0, state1[0], state1[1], state2, state3, state4};

	jive_node * bottom = jive_test_node_create(graph->root_region,
		6, tmparray3,
			tmparray4,
		1, tmparray3);
	jive_graph_export(graph, bottom->outputs[0]);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);

	jive_view(graph, stderr);

	assert(state2->node()->producer(1) == top);
	assert(state3->node()->producer(2) == top);

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-store", test_main);
