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
#include <jive/types/record.h>
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
	static const jive_record_declaration decl = {3, decl_elems};
	static jive_record_type rcdtype(&decl);

	static const jive_record_declaration decl_empty = {0, NULL};
	static jive_record_type rcdtype_empty(&decl_empty);
	const jive_type * tmparray0[] = {&bits8, &bits16, &bits32};
	
	jive_node * top = jive_node_create(graph->root_region,
		0, NULL, NULL,
		3, tmparray0);
jive_output * tmparray1[] = {top->outputs[0],
		top->outputs[1], top->outputs[2]};

	jive_output * g0 = jive_group_create(&decl, 3, tmparray1);
	jive_output * g1 = jive_empty_group_create(graph, &decl_empty);

	const jive_type * tmparray2[] = {&rcdtype, &rcdtype_empty};
	jive_output * tmparray3[] = {g0, g1};
	jive_node * bottom = jive_node_create(graph->root_region,
		2, tmparray2,
			tmparray3,
		1, tmparray0);
	jive_graph_export(graph, bottom->outputs[0]);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);

	jive_view(graph, stderr);

	assert(!jive_node_match_attrs(g0->node, jive_node_get_attrs(g1->node)));

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/record/test-rcdgroup", test_main);
