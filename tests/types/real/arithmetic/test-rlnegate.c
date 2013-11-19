/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <locale.h>

#include <jive/types/real/arithmetic/rlnegate.h>
#include <jive/types/real/rltype.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>

static int
test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	JIVE_DECLARE_REAL_TYPE(rltype);
	jive_node * top = jive_node_create(graph->root_region,
		0, NULL, NULL,
		1, &rltype);

	jive_output * neg = jive_rlnegate(top->outputs[0]);

	jive_node * bottom = jive_node_create(graph->root_region,
		1, &rltype, &neg,
		0, NULL);
	jive_node_reserve(bottom);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);
	jive_view(graph, stdout);

	jive_graph_destroy(graph);
	jive_context_assert_clean(context);
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/real/arithmetic/test-rlnegate", test_main)
