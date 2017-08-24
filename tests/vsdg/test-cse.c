/*
 * Copyright 2017 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testnodes.h"
#include "testtypes.h"

#include <jive/view.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/simple-normal-form.h>

static int
test_main()
{
	jive::test::valuetype t;
	jive::graph graph;
	auto i = graph.import(t, "i");

	auto o1 = jive::test::simple_node_create(graph.root(), {}, {}, {t})->output(0);
	auto o2 = jive::test::simple_node_create(graph.root(), {t}, {i}, {t})->output(0);

	auto e1 = graph.export_port(o1, "o1");
	auto e2 = graph.export_port(o2, "o2");

	auto nf = dynamic_cast<jive::simple_normal_form*>(graph.node_normal_form(
		typeid(jive::test::simple_op)));
	nf->set_mutable(false);

	auto o3 = jive::test::simple_node_normalized_create(graph.root(), {}, {}, {t})[0];
	auto o4 = jive::test::simple_node_normalized_create(graph.root(), {t}, {i}, {t})[0];

	auto e3 = graph.export_port(o3, "o3");
	auto e4 = graph.export_port(o4, "o4");

	nf->set_mutable(true);
	graph.normalize();
	assert(e1->origin() == e3->origin());
	assert(e2->origin() == e4->origin());

	auto o5 = jive::test::simple_node_normalized_create(graph.root(), {}, {}, {t})[0];
	assert(o5 == e1->origin());

	auto o6 = jive::test::simple_node_normalized_create(graph.root(), {t}, {i}, {t})[0];
	assert(o6 == e2->origin());

	nf->set_cse(false);

	auto o7 = jive::test::simple_node_normalized_create(graph.root(), {}, {}, {t})[0];
	assert(o7 != e1->origin());

	graph.normalize();
	assert(o7 != e1->origin());

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-cse", test_main);
