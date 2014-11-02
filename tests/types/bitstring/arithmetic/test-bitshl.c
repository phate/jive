/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/context.h>
#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph, 32, "s1");

	jive::output * c0 = jive_bitconstant_unsigned(graph, 32, 16);
	jive::output * c1 = jive_bitconstant_unsigned(graph, 32, 2);
	jive::output * c2 = jive_bitconstant_unsigned(graph, 32, 32);

	jive::output * shl0 = jive_bitshl(s0, s1);
	jive::output * shl1 = jive_bitshl(c0, c1);
	jive::output * shl2 = jive_bitshl(c0, c2);

	jive_graph_export(graph, shl0);
	jive_graph_export(graph, shl1);
	jive_graph_export(graph, shl2);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(shl0->node()->operation() == jive::bits::shl_op(32));
	assert(shl1->node()->operation() == jive::bits::uint_constant_op(32, 64));
	assert(shl2->node()->operation() == jive::bits::uint_constant_op(32, 0));

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/arithmetic/test-bitshl", test_main);
