/*
 * Copyright 2017 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/theta.h>
#include <jive/view.h>

static int
test_main()
{
	jive::graph graph;
	jive::ctl::type ctl(2);
	jive::test::valuetype t;

	auto imp1 = graph.import(ctl, "imp1");
	auto imp2 = graph.import(t, "imp2");
	auto imp3 = graph.import(t, "imp3");

	auto theta = jive::theta_node::create(graph.root());

	auto lv1 = theta->add_loopvar(imp1);
	auto lv2 = theta->add_loopvar(imp2);
	auto lv3 = theta->add_loopvar(imp3);

	lv2->result()->divert_origin(lv3->argument());
	lv3->result()->divert_origin(lv3->argument());
	theta->set_predicate(lv1->argument());

	graph.export_port(theta->output(0), "exp");
	auto theta2 = static_cast<jive::structural_node*>(theta)->copy(graph.root(), {imp1, imp2, imp3});
	jive::view(graph.root(), stdout);

	assert(lv1->output()->node() == theta);
	assert(lv2->output()->node() == theta);
	assert(lv3->output()->node() == theta);

	assert(theta->predicate() == theta->subregion()->result(0));
	assert(theta->nloopvars() == 3);
	assert(theta->begin()->result() == theta->subregion()->result(1));

	assert(dynamic_cast<const jive::theta_node*>(theta2));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("rvsdg/test-theta", test_main);
