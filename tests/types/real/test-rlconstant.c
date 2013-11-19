/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <locale.h>

#include <jive/types/real/rlconstant.h>
#include <jive/view.h>
#include <jive/vsdg.h>

static int
test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	jive_rlconstant_unsigned(graph, 9, 3);
	jive_rlconstant_unsigned(graph, 27, 9);
	jive_rlconstant_unsigned(graph, 7, 1);
	jive_rlconstant_unsigned(graph, 0, 4);
	jive_rlconstant_unsigned(graph, 2, 0);

	jive_rlconstant_signed(graph, 27, 9);
	jive_rlconstant_signed(graph, 0, 4);
	jive_rlconstant_signed(graph, 2, 0);
	jive_rlconstant_signed(graph, -1, 2);
	jive_rlconstant_signed(graph, 1, -2);
	jive_rlconstant_signed(graph, -1, -2);

	jive_rlconstant_float(graph, 0.15625);
	jive_rlconstant_float(graph, -0.0);
	jive_rlconstant_float(graph, 16.0);

	jive_rlconstant_double(graph, 16.0);

	jive_view(graph, stdout);
	jive_graph_destroy(graph);
	jive_context_assert_clean(context);
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/real/test-rlconstant", test_main)
