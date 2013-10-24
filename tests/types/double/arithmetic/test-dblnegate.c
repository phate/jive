/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <locale.h>

#include <jive/types/double/arithmetic/dblnegate.h>
#include <jive/types/double/dbltype.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>

static int
test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	JIVE_DECLARE_DOUBLE_TYPE(dbltype);
	jive_node * top = jive_node_create(graph->root_region,
		0, NULL, NULL,
		1, &dbltype);

	jive_output * neg = jive_dblnegate(top->outputs[0]);

	jive_node * bottom = jive_node_create(graph->root_region,
		1, &dbltype, &neg,
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

JIVE_UNIT_TEST_REGISTER("types/double/arithmetic/test-dblnegate", test_main)
