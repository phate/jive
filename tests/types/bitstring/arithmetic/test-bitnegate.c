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

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph, 32, "s1");

	jive::output * c0 = jive_bitconstant_unsigned(graph, 32, 3);

	jive::output * neg0 = jive_bitnegate(s0);
	jive::output * neg1 = jive_bitnegate(c0);
	jive::output * neg2 = jive_bitnegate(neg1);

	jive_graph_export(graph, neg0);
	jive_graph_export(graph, neg1);
	jive_graph_export(graph, neg2);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(neg0->node()->operation() == jive::bits::neg_op(32));
	assert(neg1->node()->operation() == jive::bits::int_constant_op(32, -3));
	assert(neg2->node()->operation() == jive::bits::int_constant_op(32, 3));

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitnegate", test_main);
