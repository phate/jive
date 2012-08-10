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
#include <jive/types/union.h>
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
	static const jive_union_declaration decl = {3, decl_elems};
	static const jive_union_type unntype = {{{&JIVE_UNION_TYPE}}, &decl};

	static const jive_union_declaration decl_empty = {0, NULL};
	static const jive_union_type unntype_empty = {{{&JIVE_UNION_TYPE}}, &decl_empty};
	
	jive_node * top = jive_node_create(graph->root_region,
		0, NULL, NULL,
		1, (const jive_type *[]){&bits8.base.base});

	jive_output * u0 = jive_unify_create(&decl, 0, top->outputs[0]);
	jive_output * u1 = jive_empty_unify_create(graph, &decl_empty);

	jive_node * bottom = jive_node_create(graph->root_region,
		2, (const jive_type *[]){&unntype.base.base, &unntype_empty.base.base},
			(jive_output *[]){u0, u1},
		0, NULL);
	jive_node_reserve(bottom);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);

	jive_view(graph, stderr);

	assert(!jive_node_match_attrs(u0->node, jive_node_get_attrs(u1->node)));

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/union/test-unnunify", test_main);
