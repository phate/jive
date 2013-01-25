/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/vsdg.h>
#include <jive/view.h>
#include <jive/types/float/fltconstant.h>

static int
test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	jive_fltconstant_float(graph, -1.0);
	jive_fltconstant_float(graph, 0.0);
	jive_fltconstant_float(graph, 1.0);
	jive_fltconstant_float(graph, 0.0 / 0.0);
	jive_fltconstant_float(graph, 1.0 / 0.0);

	jive_view(graph, stdout);
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/float/test-fltconstant", test_main);
