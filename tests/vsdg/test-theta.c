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
	tb.begin_theta(graph.root());

	auto lv1 = tb.add_loopvar(imp1);
	auto lv2 = tb.add_loopvar(imp2);
	auto lv3 = tb.add_loopvar(imp3);

	lv2->result()->divert_origin(lv3->argument());
	lv3->result()->divert_origin(lv3->argument());
	auto theta = tb.end_theta(lv1->argument());

	graph.export_port(theta->node()->output(0), "exp");

	jive::view(graph.root(), stdout);

	assert(lv1->output()->node() == theta->node());
	assert(lv2->output()->node() == theta->node());
	assert(lv3->output()->node() == theta->node());

	assert(theta->predicate() == theta->node()->subregion(0)->result(0));
	assert(theta->nloopvars() == 3);
	assert(theta->begin()->result() == theta->node()->subregion(0)->result(1));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-theta", test_main);
