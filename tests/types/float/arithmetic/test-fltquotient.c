/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/types/float.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	jive::output * s0 = jive_fltsymbolicconstant(graph, "s0");
	jive::output * s1 = jive_fltsymbolicconstant(graph, "s1");
	jive::output * div = jive_fltquotient(s0, s1);

	jive_graph_export(graph, div);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);
	jive_view(graph, stdout);

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/float/arithmetic/test-fltquotient", test_main)
