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
	jive::graph graph;

	jive::bits::type bits2(2);
	jive::bits::type bits32(32);

	auto cmp = graph.add_import(bits2, "");
	auto v0 = graph.add_import(bits32, "");
	auto v1 = graph.add_import(bits32, "");
	auto v2 = graph.add_import(bits32, "");

	auto pred = jive::ctl::match(2, {{0,0}, {1,1}}, 2, 3, cmp);

	auto gamma = jive::gamma_node::create(pred, 3);
	auto ev0 = gamma->add_entryvar(v0);
	auto ev1 = gamma->add_entryvar(v1);
	auto ev2 = gamma->add_entryvar(v2);
	gamma->add_exitvar({ev0->argument(0), ev1->argument(1), ev2->argument(2)});

	graph.export_port(gamma->output(0), "dummy");
	auto gamma2 = static_cast<jive::structural_node*>(gamma)->copy(graph.root(), {pred, v0, v1, v2});
	jive::view(graph.root(), stdout);

	assert(gamma && gamma->operation() == jive::gamma_op(3));
	assert(dynamic_cast<const jive::gamma_node*>(gamma2));
}

static void
test_predicate_reduction(void)
{
	jive::graph graph;
	jive::gamma_op::normal_form(&graph)->set_predicate_reduction(true);

	jive::bits::type bits2(2);
	jive::bits::type bits32(32);

	auto v0 = graph.add_import(bits32, "");
	auto v1 = graph.add_import(bits32, "");
	auto v2 = graph.add_import(bits32, "");

	auto pred = jive_control_constant(graph.root(), 3, 1);

	auto gamma = jive::gamma_node::create(pred, 3);
	auto ev0 = gamma->add_entryvar(v0);
	auto ev1 = gamma->add_entryvar(v1);
	auto ev2 = gamma->add_entryvar(v2);
	gamma->add_exitvar({ev0->argument(0), ev1->argument(1), ev2->argument(2)});

	auto r = graph.export_port(gamma->output(0), "");

	graph.normalize();
//	jive::view(graph.root(), stdout);
	assert(r->origin() == v1);

	graph.prune();
	assert(graph.root()->nnodes() == 0);
}

static void
test_invariant_reduction(void)
{
	jive::graph graph;
	jive::gamma_op::normal_form(&graph)->set_invariant_reduction(true);

	jive::test::valuetype vtype;

	auto pred = graph.add_import(jive::ctl::boolean, "");
	auto v = graph.add_import(vtype, "");

	auto gamma = jive::gamma_node::create(pred, 2);
	auto ev = gamma->add_entryvar(v);
	gamma->add_exitvar({ev->argument(0), ev->argument(1)});

	auto r = graph.export_port(gamma->output(0), "");

	graph.normalize();
//	jive::view(graph.root(), stdout);
	assert(r->origin() == v);

	graph.prune();
	assert(graph.root()->nnodes() == 0);
}

static void
test_control_constant_reduction()
{
	jive::bits::type bt(1);

	jive::graph graph;
	jive::gamma_op::normal_form(&graph)->set_control_constant_reduction(true);

	auto x = graph.add_import(bt, "x");

	auto c = jive::ctl::match(1, {{0, 0}}, 1, 2, x);

	auto gamma = jive::gamma_node::create(c, 2);

	auto t = jive_control_true(gamma->subregion(0));
	auto f = jive_control_false(gamma->subregion(1));

	gamma->add_exitvar({t, f});

	auto ex = graph.export_port(gamma->output(0), "c");

	jive::view(graph.root(), stdout);
	graph.normalize();
	jive::view(graph.root(), stdout);

	auto match = ex->origin()->node();
	assert(match && jive::is_match_op(match->operation()));
	auto & match_op = jive::to_match_op(match->operation());
	assert(match_op.default_alternative() == 0);
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

JIVE_UNIT_TEST_REGISTER("rvsdg/test-gamma", test_main);
