/*
 * Copyright 2017 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <jive/view.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/theta.h>

static int
test_main()
{
	jive::graph graph;
	jive::ctl::type ctl(2);
	jive::test::valuetype t;

	auto imp1 = graph.import(ctl, "imp1");
	auto imp2 = graph.import(t, "imp2");
	auto imp3 = graph.import(t, "imp3");

	jive::theta_builder tb;
	tb.begin(graph.root());

	auto lv1 = tb.add_loopvar(imp1);
	auto lv2 = tb.add_loopvar(imp2);
	auto lv3 = tb.add_loopvar(imp3);

	auto theta = tb.end(lv1->argument(), {{lv2, lv3->argument()}, {lv3, lv3->argument()}});

	graph.export_port(theta->output(0), "exp");

	jive::view(graph.root(), stdout);

	assert(lv1->output()->node() == theta);
	assert(lv2->output()->node() == theta);
	assert(lv3->output()->node() == theta);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-theta", test_main);
