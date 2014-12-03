/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/types/float/fltconstant.h>
#include <jive/view.h>
#include <jive/vsdg.h>

static int
test_main(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive_fltconstant_float(graph, -1.0);
	jive_fltconstant_float(graph, 0.0);
	jive_fltconstant_float(graph, 1.0);
	jive_fltconstant_float(graph, 0.0 / 0.0);
	jive_fltconstant_float(graph, 1.0 / 0.0);

	jive_view(graph, stdout);
	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/float/test-fltconstant", test_main);
