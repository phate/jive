/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>
#include <stdio.h>

#include <jive/rvsdg.h>
#include <jive/view.h>

#include "testnodes.h"

static bool
region_contains_node(const jive::region * region, const jive::node * n)
{
	for (const auto & node : region->nodes) {
		if (&node == n)
			return true;
	}

	return false;
}

static int
test_recursive_prune()
{
	jive::graph graph;
	jive::test::valuetype t;
	auto imp = graph.import(t, "i");

	auto n1 = jive::test::simple_node_create(graph.root(), {t}, {imp}, {t});
	auto n2 = jive::test::simple_node_create(graph.root(), {t}, {imp}, {t});

	auto n3 = jive::test::structural_node_create(graph.root(), 1);
	n3->add_input(t, imp);
	auto a1 = n3->subregion(0)->add_argument(nullptr, t);
	auto n4 = jive::test::simple_node_create(n3->subregion(0), {t}, {a1}, {t});
	auto n5 = jive::test::simple_node_create(n3->subregion(0), {t}, {a1}, {t});
	n3->subregion(0)->add_result(n4->output(0), nullptr, t);
	auto o1 = n3->add_output(t);

	auto n6 = jive::test::structural_node_create(n3->subregion(0), 1);

	graph.export_port(n2->output(0), "n2");
	graph.export_port(o1, "n3");

	jive::view(graph.root(), stdout);
	graph.prune();
	jive::view(graph.root(), stdout);

	assert(!region_contains_node(graph.root(), n1));
	assert(region_contains_node(graph.root(), n2));
	assert(region_contains_node(graph.root(), n3));
	assert(region_contains_node(n3->subregion(0), n4));
	assert(!region_contains_node(n3->subregion(0), n5));
	assert(!region_contains_node(n3->subregion(0), n6));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("rvsdg/test-graph_prune", test_recursive_prune);

static int
test_empty_graph_pruning(void)
{
	jive::graph graph;

	jive::view(graph.root(), stdout);

	graph.prune();

	assert(graph.root()->nnodes() == 0);

	jive::view(graph.root(), stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("rvsdg/test-empty_graph_pruning", test_empty_graph_pruning);

static int
test_prune_replace(void)
{
	jive::graph graph;

	jive::region * region = graph.root();
	jive::test::valuetype type;
	auto n1 = jive::test::simple_node_create(region, {}, {}, {type});
	auto n2 = jive::test::simple_node_create(region, {type}, {n1->output(0)}, {type});
	auto n3 = jive::test::simple_node_create(region, {type}, {n2->output(0)}, {type});

	graph.export_port(n2->output(0), "n2");
	graph.export_port(n3->output(0), "n3");

	auto n4 = jive::test::simple_node_create(region, {type}, {n1->output(0)}, {type});

	n2->output(0)->replace(n4->output(0));
	assert(n2->output(0)->nusers() == 0);

	graph.prune();

	assert(!region_contains_node(region, n2));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("rvsdg/test-prune-replace", test_prune_replace);

static int
test_graph(void)
{
	jive::graph graph;
	
	jive::region * region = graph.root();
	
	jive::test::valuetype type;
	auto n1 = jive::test::simple_node_create(region, {}, {}, {type});
	assert(n1);
	assert(n1->depth() == 0);

	auto n2 = jive::test::simple_node_create(region, {type}, {n1->output(0)}, {});
	assert(n2);
	assert(n2->depth() == 1);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("rvsdg/test-graph", test_graph);
