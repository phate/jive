/*
 * Copyright 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/types/bitstring.h>
#include <jive/types/record.h>
#include <jive/vsdg/node-private.h>
#include <jive/arch/addresstype.h>
#include <jive/arch/load.h>

static int test_main()
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	static const jive_bitstring_type bits8 = {{{&JIVE_BITSTRING_TYPE}}, 8};
	static const jive_bitstring_type bits16 = {{{&JIVE_BITSTRING_TYPE}}, 16};
	static const jive_bitstring_type bits32 = {{{&JIVE_BITSTRING_TYPE}}, 32};

	static const jive_value_type * decl_elems[] = {&bits8.base, &bits16.base, &bits32.base};
	static const jive_record_declaration decl = {3, decl_elems};
	static const jive_record_type rcdtype = {{{&JIVE_RECORD_TYPE}}, &decl};

	JIVE_DECLARE_ADDRESS_TYPE(addrtype);
	jive_node * top = jive_node_create(graph->root_region,
		0, NULL, NULL,
		6, (const jive_type *[]){&bits8.base.base, &bits16.base.base, &bits32.base.base,
			&rcdtype.base.base, &rcdtype.base.base, addrtype});

	jive_output * g0 = jive_group_create(&decl, 3, (jive_output *[]){top->outputs[0],
		top->outputs[1], top->outputs[2]});
	jive_output * load = jive_load_by_address_create(top->outputs[5], &rcdtype.base,
		0, NULL);

	jive_output * s0 = jive_select_create(1, top->outputs[3]);
	jive_output * s1 = jive_select_create(1, g0);
	jive_output * s2 = jive_select_create(2, top->outputs[4]);
	jive_output * s3 = jive_select_create(0, load);

	jive_node * bottom = jive_node_create(graph->root_region,
		4, (const jive_type *[]){&bits16.base.base, &bits16.base.base, &bits32.base.base,
			&bits8.base.base}, (jive_output *[]){s0, s1, s2, s3},
		0, NULL);
	jive_node_reserve(bottom);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);

	jive_view(graph, stderr);

	assert(bottom->inputs[1]->origin->node == top);
	assert(!jive_node_match_attrs(s0->node, jive_node_get_attrs(s2->node)));
	assert(jive_node_isinstance(bottom->inputs[3]->origin->node, &JIVE_LOAD_NODE));

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/record/test-rcdselect", test_main);
