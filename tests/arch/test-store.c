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

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);
	
	static const jive_bitstring_type bits8 = {{{&JIVE_BITSTRING_TYPE}}, 8};
	static const jive_bitstring_type bits16 = {{{&JIVE_BITSTRING_TYPE}}, 16};
	static const jive_bitstring_type bits32 = {{{&JIVE_BITSTRING_TYPE}}, 32};

	static const jive_value_type * decl_elems[] = {&bits8.base, &bits16.base, &bits32.base};
	static const jive_record_declaration rcddecl = {3, decl_elems};
	static const jive_record_type rcdtype = {{{&JIVE_RECORD_TYPE}}, &rcddecl};
	
	static const jive_union_declaration unndecl = {3, decl_elems};
	static const jive_union_type unntype = {{{&JIVE_UNION_TYPE}}, &unndecl};

	static const jive_union_declaration empty_unndecl = {0, NULL};
	static const jive_union_type empty_unntype = {{{&JIVE_UNION_TYPE}}, &empty_unndecl};

	JIVE_DECLARE_MEMORY_TYPE(memtype);
	JIVE_DECLARE_ADDRESS_TYPE(addrtype);
	jive_node * top = jive_node_create(graph->root_region,
		0, NULL, NULL,
		6, (const jive_type *[]){addrtype, memtype, &bits8.base.base, &bits16.base.base,
			&bits32.base.base, memtype});

	jive_output * state0;
	jive_store_by_address_create(top->outputs[0], &bits32.base, top->outputs[4],
		1, &top->outputs[1], &state0);

	jive_output * state1[2];
	jive_output * group = jive_group_create(&rcddecl, 3, (jive_output *[]){top->outputs[2],
		top->outputs[3], top->outputs[4]});
	jive_store_by_address_create(top->outputs[0], &rcdtype.base, group,
		2, (jive_output *[]){top->outputs[1], top->outputs[5]}, state1);

	jive_output * state2;
	jive_output * unify = jive_unify_create(&unndecl, 2, top->outputs[4]);
	jive_store_by_address_create(top->outputs[0], &unntype.base, unify,
		1, &top->outputs[1], &state2);

	jive_output * state3;
	jive_store_by_address_create(top->outputs[0], &bits32.base, top->outputs[4],
		1, &top->outputs[1], &state3);
	jive_store_by_address_create(top->outputs[0], &bits32.base, top->outputs[4],
		1, &state3, &state3);

	jive_output * state4;
	unify = jive_empty_unify_create(graph, &empty_unndecl);
	jive_store_by_address_create(top->outputs[0], &empty_unntype.base, unify,
		1, &top->outputs[1], &state4);

	jive_node * bottom = jive_node_create(graph->root_region,
		6, (const jive_type *[]){memtype, memtype, memtype, memtype, memtype, memtype},
			(jive_output *[]){state0, state1[0], state1[1], state2, state3, state4},
		0, NULL);	
	jive_node_reserve(bottom);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);

	jive_view(graph, stderr);

	assert(state2->node->inputs[1]->origin->node == top);
	assert(state3->node->inputs[2]->origin->node == top);

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-store", test_main);
