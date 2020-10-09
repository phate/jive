/*
 * Copyright 2017 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.hpp"
#include "testnodes.hpp"
#include "testtypes.hpp"

#include <jive/rvsdg/graph.hpp>
#include <jive/rvsdg/simple-normal-form.hpp>
#include <jive/view.hpp>

static int
test_main()
{
	using namespace jive;

	test::valuetype t;

	jive::graph graph;
	auto i = graph.add_import({t, "i"});

	auto o1 = test::simple_node_create(graph.root(), {}, {}, {t})->output(0);
	auto o2 = test::simple_node_create(graph.root(), {t}, {i}, {t})->output(0);

	auto e1 = graph.add_export(o1, {o1->type(), "o1"});
	auto e2 = graph.add_export(o2, {o2->type(), "o2"});

	auto nf = dynamic_cast<jive::simple_normal_form*>(graph.node_normal_form(
		typeid(test::simple_op)));
	nf->set_mutable(false);

	auto o3 = test::simple_node_normalized_create(graph.root(), {}, {}, {t})[0];
	auto o4 = test::simple_node_normalized_create(graph.root(), {t}, {i}, {t})[0];

	auto e3 = graph.add_export(o3, {o3->type(), "o3"});
	auto e4 = graph.add_export(o4, {o4->type(), "o4"});

	nf->set_mutable(true);
	graph.normalize();
	assert(e1->origin() == e3->origin());
	assert(e2->origin() == e4->origin());

	auto o5 = test::simple_node_normalized_create(graph.root(), {}, {}, {t})[0];
	assert(o5 == e1->origin());

	auto o6 = test::simple_node_normalized_create(graph.root(), {t}, {i}, {t})[0];
	assert(o6 == e2->origin());

	nf->set_cse(false);

	auto o7 = test::simple_node_normalized_create(graph.root(), {}, {}, {t})[0];
	assert(o7 != e1->origin());

	graph.normalize();
	assert(o7 != e1->origin());

	return 0;
}

JIVE_UNIT_TEST_REGISTER("rvsdg/test-cse", test_main)
