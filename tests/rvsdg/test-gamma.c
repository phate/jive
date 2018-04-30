/*
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testnodes.h"
#include "testtypes.h"

#include <jive/rvsdg/control.h>
#include <jive/rvsdg/gamma.h>
#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/substitution.h>
#include <jive/types/bitstring/type.h>
#include <jive/view.h>

static void
test_gamma(void)
{
	using namespace jive;

	bittype bit2(2);

	jive::graph graph;
	auto cmp = graph.add_import(bit2, "");
	auto v0 = graph.add_import(bit32, "");
	auto v1 = graph.add_import(bit32, "");
	auto v2 = graph.add_import(bit32, "");
	auto v3 = graph.add_import(ctl2, "");

	auto pred = match(2, {{0,0}, {1,1}}, 2, 3, cmp);

	auto gamma = gamma_node::create(pred, 3);
	auto ev0 = gamma->add_entryvar(v0);
	auto ev1 = gamma->add_entryvar(v1);
	auto ev2 = gamma->add_entryvar(v2);
	gamma->add_exitvar({ev0->argument(0), ev1->argument(1), ev2->argument(2)});

	graph.add_export(gamma->output(0), "dummy");

	assert(gamma && gamma->operation() == jive::gamma_op(3));

	/* test gamma copy */

	auto gamma2 = static_cast<structural_node*>(gamma)->copy(graph.root(), {pred, v0, v1, v2});
	view(graph.root(), stdout);
	assert(is<gamma_op>(gamma2));

	/* test entry and exit variable iterators */

	auto gamma3 = gamma_node::create(v3, 2);
	assert(gamma3->begin_entryvar() == gamma3->end_entryvar());
	assert(gamma3->begin_exitvar() == gamma3->end_exitvar());
}

static void
test_predicate_reduction(void)
{
	using namespace jive;

	jive::graph graph;
	gamma_op::normal_form(&graph)->set_predicate_reduction(true);

	bittype bits2(2);

	auto v0 = graph.add_import(bit32, "");
	auto v1 = graph.add_import(bit32, "");
	auto v2 = graph.add_import(bit32, "");

	auto pred = jive_control_constant(graph.root(), 3, 1);

	auto gamma = gamma_node::create(pred, 3);
	auto ev0 = gamma->add_entryvar(v0);
	auto ev1 = gamma->add_entryvar(v1);
	auto ev2 = gamma->add_entryvar(v2);
	gamma->add_exitvar({ev0->argument(0), ev1->argument(1), ev2->argument(2)});

	auto r = graph.add_export(gamma->output(0), "");

	graph.normalize();
//	jive::view(graph.root(), stdout);
	assert(r->origin() == v1);

	graph.prune();
	assert(graph.root()->nnodes() == 0);
}

static void
test_invariant_reduction(void)
{
	using namespace jive;

	test::valuetype vtype;

	jive::graph graph;
	gamma_op::normal_form(&graph)->set_invariant_reduction(true);

	auto pred = graph.add_import(ctl2, "");
	auto v = graph.add_import(vtype, "");

	auto gamma = jive::gamma_node::create(pred, 2);
	auto ev = gamma->add_entryvar(v);
	gamma->add_exitvar({ev->argument(0), ev->argument(1)});

	auto r = graph.add_export(gamma->output(0), "");

	graph.normalize();
//	jive::view(graph.root(), stdout);
	assert(r->origin() == v);

	graph.prune();
	assert(graph.root()->nnodes() == 0);
}

static void
test_control_constant_reduction()
{
	using namespace jive;

	jive::graph graph;
	gamma_op::normal_form(&graph)->set_control_constant_reduction(true);

	auto x = graph.add_import(bit1, "x");

	auto c = match(1, {{0, 0}}, 1, 2, x);

	auto gamma = gamma_node::create(c, 2);

	auto t = jive_control_true(gamma->subregion(0));
	auto f = jive_control_false(gamma->subregion(1));

	auto n0 = jive_control_constant(gamma->subregion(0), 3, 0);
	auto n1 = jive_control_constant(gamma->subregion(1), 3, 1);

	auto xv1 = gamma->add_exitvar({t, f});
	auto xv2 = gamma->add_exitvar({n0, n1});

	auto ex1 = graph.add_export(xv1, "");
	auto ex2 = graph.add_export(xv2, "");

	jive::view(graph.root(), stdout);
	graph.normalize();
	jive::view(graph.root(), stdout);

	auto match = ex1->origin()->node();
	assert(match && is<match_op>(match->operation()));
	auto & match_op = to_match_op(match->operation());
	assert(match_op.default_alternative() == 0);

	assert(ex2->origin()->node() == gamma);
}

static int
test_main(void)
{
	test_gamma();
	test_predicate_reduction();
	test_invariant_reduction();
	test_control_constant_reduction();

	return 0;
}

JIVE_UNIT_TEST_REGISTER("rvsdg/test-gamma", test_main)
