/*
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testnodes.h"
#include "testtypes.h"

#include <jive/types/bitstring/type.h>
#include <jive/view.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/gamma.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/simple_node.h>

static void
test_gamma(void)
{
	jive::graph graph;

	jive::bits::type bits2(2);
	jive::bits::type bits32(32);

	auto cmp = graph.import(bits2, "");
	auto v0 = graph.import(bits32, "");
	auto v1 = graph.import(bits32, "");
	auto v2 = graph.import(bits32, "");

	auto pred = jive::ctl::match(2, {{0,0}, {1,1}}, 2, 3, cmp);

	jive::gamma_builder gb;
	gb.begin(pred);
	auto ev0 = gb.add_entryvar(v0);
	auto ev1 = gb.add_entryvar(v1);
	auto ev2 = gb.add_entryvar(v2);
	gb.add_exitvar({ev0->argument(0), ev1->argument(1), ev2->argument(2)});
	auto gamma = gb.end();

	graph.export_port(gamma->node()->output(0), "dummy");
	jive::view(graph.root(), stdout);

	assert(gamma && gamma->node()->operation() == jive::gamma_op(3));
}

static void
test_predicate_reduction(void)
{
	jive::graph graph;
	jive::gamma_op::normal_form(&graph)->set_predicate_reduction(true);

	jive::bits::type bits2(2);
	jive::bits::type bits32(32);

	auto v0 = graph.import(bits32, "");
	auto v1 = graph.import(bits32, "");
	auto v2 = graph.import(bits32, "");

	auto pred = jive_control_constant(graph.root(), 3, 1);

	jive::gamma_builder gb;
	gb.begin(pred);
	auto ev0 = gb.add_entryvar(v0);
	auto ev1 = gb.add_entryvar(v1);
	auto ev2 = gb.add_entryvar(v2);
	gb.add_exitvar({ev0->argument(0), ev1->argument(1), ev2->argument(2)});
	auto gamma = gb.end();

	auto r = graph.export_port(gamma->node()->output(0), "");

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

	auto pred = graph.import(jive::ctl::boolean, "");
	auto v = graph.import(vtype, "");

	jive::gamma_builder gb;
	gb.begin(pred);
	auto ev = gb.add_entryvar(v);
	gb.add_exitvar({ev->argument(0), ev->argument(1)});
	auto gamma = gb.end();

	auto r = graph.export_port(gamma->node()->output(0), "");

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

	auto x = graph.import(bt, "x");

	auto c = jive::ctl::match(1, {{0, 0}}, 1, 2, x);

	jive::gamma_builder gb;
	gb.begin(c);

	auto t = jive_control_true(gb.subregion(0));
	auto f = jive_control_false(gb.subregion(1));

	gb.add_exitvar({t, f});

	auto gamma = gb.end();

	auto ex = graph.export_port(gamma->node()->output(0), "c");

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

JIVE_UNIT_TEST_REGISTER("vsdg/test-gamma", test_main);
