/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <locale.h>

#include <jive/types/double/comparison/dblgreater.h>
#include <jive/types/double/dbltype.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	JIVE_DECLARE_CONTROL_TYPE(ctype);
	JIVE_DECLARE_DOUBLE_TYPE(dbltype);
	const jive_type* tmparray0[] = {dbltype, dbltype};
	jive_node * top = jive_node_create(graph->root_region,
		0, NULL, NULL,
		2, tmparray0);

	jive_output * greater = jive_dblgreater(top->outputs[0], top->outputs[1]);

	jive_node * bottom = jive_node_create(graph->root_region,
		1, &ctype, &greater,
		1, &dbltype);
	jive_graph_export(graph, bottom->outputs[0]);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);
	jive_view(graph, stdout);

	jive_graph_destroy(graph);
	jive_context_assert_clean(context);
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/double/comparison/test-dblgreater", test_main)
