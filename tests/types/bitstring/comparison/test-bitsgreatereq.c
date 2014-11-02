/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include "bitcmp-test-helpers.h"

#include <assert.h>
#include <locale.h>
#include <stdint.h>

#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/node-private.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::output * s0 = jive_bitsymbolicconstant(graph, 32, "s0");
	jive::output * s1 = jive_bitsymbolicconstant(graph, 32, "s1");
	jive::output * c0 = jive_bitconstant_signed(graph, 32, 4);
	jive::output * c1 = jive_bitconstant_signed(graph, 32, 5);
	jive::output * c2 = jive_bitconstant_signed(graph, 32, 0x7fffffffL);
	jive::output * c3 = jive_bitconstant_signed(graph, 32, (-0x7fffffffL-1));

	jive::output * sgreatereq0 = jive_bitsgreatereq(s0, s1);
	jive::output * sgreatereq1 = jive_bitsgreatereq(c0, c1);
	jive::output * sgreatereq2 = jive_bitsgreatereq(c1, c0);
	jive::output * sgreatereq3 = jive_bitsgreatereq(c0, c0);
	jive::output * sgreatereq4 = jive_bitsgreatereq(c2, s0);
	jive::output * sgreatereq5 = jive_bitsgreatereq(s1, c3);

	jive_graph_export(graph, sgreatereq0);
	jive_graph_export(graph, sgreatereq1);
	jive_graph_export(graph, sgreatereq2);
	jive_graph_export(graph, sgreatereq3);
	jive_graph_export(graph, sgreatereq4);
	jive_graph_export(graph, sgreatereq5);

	jive_graph_prune(graph);
	jive_view(graph, stdout);

	assert(sgreatereq0->node()->operation() == jive::bits::sge_op(32));
	expect_static_false(sgreatereq1);
	expect_static_true(sgreatereq2);
	expect_static_true(sgreatereq3);
	expect_static_true(sgreatereq4);
	expect_static_true(sgreatereq5);

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/comparison/test-bitsgreatereq", test_main);
