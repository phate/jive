/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	jive::output * s0 = jive_bitsymbolicconstant(graph, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph, 32, "s1");

	jive::output * c0 = jive_bitconstant_unsigned(graph, 32, 3);
	jive::output * c1 = jive_bitconstant_unsigned(graph, 32, 5);
	jive::output * tmparray1[] = {s0, s1};

	jive::output * or0 = jive_bitor(2, tmparray1);
	jive::output * tmparray2[] = {c0, c1};
	jive::output * or1 = jive_bitor(2, tmparray2);

	jive_graph_export(graph, or0);
	jive_graph_export(graph, or1);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(or0->node()->operation() == jive::bits::or_op(32));
	assert(or1->node()->operation() == jive::bits::uint_constant_op(32, 7));

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitor", test_main);
