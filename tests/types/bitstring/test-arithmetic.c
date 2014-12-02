/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * c0 = jive_bitconstant(graph, 4, "1100");
	jive::output * c1 = jive_bitconstant(graph, 4, "0001");

	jive_bitdifference(c0, c1);
	jive_bitshiproduct(c0, c1);
	jive_bituhiproduct(c0, c1);
	jive_bituquotient(c0, c1);
	jive_bitsquotient(c0, c1);
	jive_bitumod(c0, c1);
	jive_bitsmod(c0, c1);
	jive_bitshl(c0, c1);
	jive_bitshr(c0, c1);
	jive_bitashr(c0, c1);

	jive_view(graph, stdout);

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/test-arithmetic", test_main);
