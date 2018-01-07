/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>

#include <jive/rvsdg.h>
#include <jive/rvsdg/statemux.h>
#include <jive/view.h>

#include "testnodes.h"

static void
test_mux_mux_reduction()
{
	jive::test::statetype st;

	jive::graph graph;
	auto nf = graph.node_normal_form(typeid(jive::mux_op));
	auto mnf = static_cast<jive::mux_normal_form*>(nf);
	mnf->set_mutable(false);
	mnf->set_mux_mux_reducible(false);

	auto x = graph.add_import(st, "x");
	auto y = graph.add_import(st, "y");
	auto z = graph.add_import(st, "z");

	auto mux1 = jive::create_state_merge(st, {x, y});
	auto mux2 = jive::create_state_split(st, z, 2);
	auto mux3 = jive::create_state_merge(st, {mux1, mux2[0], mux2[1], z});

	auto ex = graph.add_export(mux3, "m");

//	jive::view(graph.root(), stdout);

	mnf->set_mutable(true);
	mnf->set_mux_mux_reducible(true);
	graph.normalize();
	graph.prune();

//	jive::view(graph.root(), stdout);

	auto node = ex->origin()->node();
	assert(node->ninputs() == 4);
	assert(node->input(0)->origin() == x);
	assert(node->input(1)->origin() == y);
	assert(node->input(2)->origin() == z);
	assert(node->input(3)->origin() == z);
}

static void
test_multiple_origin_reduction()
{
	jive::test::statetype st;

	jive::graph graph;
	auto nf = graph.node_normal_form(typeid(jive::mux_op));
	auto mnf = static_cast<jive::mux_normal_form*>(nf);
	mnf->set_mutable(false);
	mnf->set_multiple_origin_reducible(false);

	auto x = graph.add_import(st, "x");
	auto mux1 = jive::create_state_merge(st, {x, x});
	auto ex = graph.add_export(mux1, "m");

	jive::view(graph.root(), stdout);

	mnf->set_mutable(true);
	mnf->set_multiple_origin_reducible(true);
	graph.normalize();
	graph.prune();

	jive::view(graph.root(), stdout);

	assert(ex->origin()->node()->ninputs() == 1);
}

static int
test_main(void)
{
	test_mux_mux_reduction();
	test_multiple_origin_reduction();

	return 0;
}

JIVE_UNIT_TEST_REGISTER("rvsdg/test-statemux", test_main)
