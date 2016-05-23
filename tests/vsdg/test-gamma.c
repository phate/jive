/*
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testnodes.h"

#include <locale.h>

#include <jive/types/bitstring/type.h>
#include <jive/view.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/gamma.h>
#include <jive/vsdg/graph.h>

static int
test_main(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::bits::type bits2(2);
	jive::bits::type bits32(32);
	jive_node * top = jive_test_node_create(graph->root_region,
		{}, {}, {&bits2, &bits32, &bits32, &bits32});

	jive::output * cmp = top->outputs[0];
	jive::output * v0 = top->outputs[1];
	jive::output * v1 = top->outputs[2];
	jive::output * v2 = top->outputs[3];

	std::vector<jive::output*> result;

	//create normal gamma
	jive::output * pred = jive::ctl::match(2, {{0,0}, {1,1}}, 2, 3, cmp);
	result = jive_gamma(pred, {&bits32}, {{v0}, {v1}, {v2}});
	jive_graph_export(graph, result[0]);
	assert(result[0]->node()->operation() == jive::gamma_op(3));

	//predicate reduction
	pred = jive_control_constant(graph->root_region, 3, 1);
	result = jive_gamma(pred, {&bits32}, {{v0}, {v1}, {v2}});
	jive_graph_export(graph, result[0]);
	assert(result[0] == v1);

	//invariant variable reduction
	pred = jive::ctl::match(2, {{0,0}, {1,1}}, 2, 3, cmp);
	result = jive_gamma(pred, {&bits32, &bits32}, {{v0, v0}, {v0, v1}, {v0, v2}});
	jive_graph_export(graph, result[0]);
	jive_graph_export(graph, result[1]);
	assert(result[0] == v0);
	assert(result[1]->node()->operation() == jive::gamma_op(3));

	jive_view(graph, stdout);

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-gamma", test_main);
