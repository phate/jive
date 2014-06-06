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
#include <jive/arch/addresstype.h>
#include <jive/arch/load.h>

#include "testnodes.h"

static int test_main()
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	static const jive::bits::type bits8(8);
	static const jive::bits::type bits16(16);
	static const jive::bits::type bits32(32);

	static const jive::value::type * decl_elems[] = {&bits8, &bits16, &bits32};
	static const jive::rcd::declaration decl = {3, decl_elems};
	static jive::rcd::type rcdtype(&decl);

	jive::addr::type addrtype;
	const jive::base::type * tmparray0[] = {&bits8, &bits16, &bits32, &rcdtype, &rcdtype, &addrtype};
	jive_node * top = jive_test_node_create(graph->root_region,
		0, NULL, NULL,
		6, tmparray0);
	jive_output * tmparray1[] = {top->outputs[0],
		top->outputs[1], top->outputs[2]};

	jive_output * g0 = jive_group_create(&decl, 3, tmparray1);
	jive_output * load = jive_load_by_address_create(top->outputs[5], &rcdtype, 0, NULL);

	jive_output * s0 = jive_select_create(1, top->outputs[3]);
	jive_output * s1 = jive_select_create(1, g0);
	jive_output * s2 = jive_select_create(2, top->outputs[4]);
	jive_output * s3 = jive_select_create(0, load);
	const jive::base::type * tmparray2[] = {&bits16, &bits16, &bits32, &bits8};
	jive_output * tmparray3[] = {s0, s1, s2, s3};

	jive_node * bottom = jive_test_node_create(graph->root_region,
		4, tmparray2, tmparray3,
		1, tmparray0);
	jive_graph_export(graph, bottom->outputs[0]);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);

	jive_view(graph, stderr);

	assert(bottom->producer(1) == top);
	assert(!jive_node_match_attrs(s0->node(), jive_node_get_attrs(s2->node())));
	assert(jive_node_isinstance(bottom->producer(3), &JIVE_LOAD_NODE));

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/record/test-rcdselect", test_main);
