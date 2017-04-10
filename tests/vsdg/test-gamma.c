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

	auto cmp = graph.import(bits2, "");
	auto v0 = graph.import(bits32, "");
	auto v1 = graph.import(bits32, "");
	auto v2 = graph.import(bits32, "");

	//create normal gamma
	auto pred = jive::ctl::match(2, {{0,0}, {1,1}}, 2, 3, cmp);

	jive::gamma_builder gb;
	gb.begin(pred);
	auto ev0 = gb.add_entryvar(v0);
	auto ev1 = gb.add_entryvar(v1);
	auto ev2 = gb.add_entryvar(v2);
	gb.add_exitvar({ev0->argument(0), ev1->argument(1), ev2->argument(2)});
	auto gamma = gb.end();

	graph.export_port(gamma->output(0), "dummy");
	assert(gamma && gamma->operation() == jive::gamma_op(3));

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
