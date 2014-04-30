/*
 * Copyright 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
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
	
	static const jive_bitstring_type bits8(8);
	static const jive_bitstring_type bits16(16);
	static const jive_bitstring_type bits32(32);

	static const jive_value_type * decl_elems[] = {&bits8, &bits16, &bits32};
	static const jive_union_declaration decl = {3, decl_elems};
	static jive_union_type unntype(&decl);

	static const jive_union_declaration decl_empty = {0, NULL};
	static jive_union_type unntype_empty(&decl_empty);
	const jive_type * tmparray0[] = {&bits8};
	
	jive_node * top = jive_node_create(graph->root_region,
		0, NULL, NULL,
		1, tmparray0);

	jive_output * u0 = jive_unify_create(&decl, 0, top->outputs[0]);
	jive_output * u1 = jive_empty_unify_create(graph, &decl_empty);

	const jive_type * tmparray1[] = {&unntype, &unntype_empty};
	jive_output * tmparray2[] = {u0, u1};
	jive_node * bottom = jive_node_create(graph->root_region,
		2, tmparray1,
			tmparray2,
		1, tmparray0);
	jive_graph_export(graph, bottom->outputs[0]);

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
