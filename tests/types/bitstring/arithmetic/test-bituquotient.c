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

	jive::output * c0 = jive_bitconstant_unsigned(graph, 32, 7);
	jive::output * c1 = jive_bitconstant_unsigned(graph, 32, 3);

	jive::output * uquot0 = jive_bituquotient(s0, s1);
	jive::output * uquot1 = jive_bituquotient(c0, c1);

	jive_graph_export(graph, uquot0);
	jive_graph_export(graph, uquot1);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(uquot0->node()->operation() == jive::bits::udiv_op(32));
	assert(uquot1->node()->operation() == jive::bits::int_constant_op(32, 2));

	jive_graph_destroy(graph);

	return 0;
}


JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bituquotient", test_main);
