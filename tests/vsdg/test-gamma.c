/*
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testnodes.h"

#include <jive/types/bitstring/type.h>
#include <jive/view.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/gamma.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/simple_node.h>

static int
test_main(void)
{
	jive::graph graph;

	jive::bits::type bits2(2);
	jive::bits::type bits32(32);
	jive::node * top = jive_test_node_create(graph.root(),
		{}, {}, {&bits2, &bits32, &bits32, &bits32});

	jive::output * cmp = dynamic_cast<jive::output*>(top->output(0));
	jive::output * v0 = dynamic_cast<jive::output*>(top->output(1));
	jive::output * v1 = dynamic_cast<jive::output*>(top->output(2));
	jive::output * v2 = dynamic_cast<jive::output*>(top->output(3));

	//create normal gamma
	auto pred = jive::ctl::match(2, {{0,0}, {1,1}}, 2, 3, cmp);
	auto result = jive_gamma(pred, {&bits32}, {{v0}, {v1}, {v2}});
	graph.export_port(result[0], "dummy");
	assert(result[0]->node() && result[0]->node()->operation() == jive::gamma_op(3));

	jive::view(graph.root(), stdout);
#if 0
	//predicate reduction
	pred = jive_control_constant(graph.root(), 3, 1);
	result = jive_gamma(pred, {&bits32}, {{v0}, {v1}, {v2}});
	jive_graph_export(&graph, result[0]);
	assert(result[0] == v1);

	//invariant variable reduction
	pred = jive::ctl::match(2, {{0,0}, {1,1}}, 2, 3, cmp);
	result = jive_gamma(pred, {&bits32, &bits32}, {{v0, v0}, {v0, v1}, {v0, v2}});
	jive_graph_export(&graph, result[0]);
	jive_graph_export(&graph, result[1]);
	assert(result[0] == v0);
	assert(result[1]->node()->operation() == jive::gamma_op(3));
#endif
//	jive_view(&graph, stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-gamma", test_main);
