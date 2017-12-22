/*
 * Copyright 2017 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testnodes.h"
#include "testtypes.h"

#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/substitution.h>
#include <jive/view.h>

static void
test_node_copy(void)
{
	jive::test::statetype stype;
	jive::test::valuetype vtype;

	jive::graph graph;
	auto s = graph.add_import(stype, "");
	auto v = graph.add_import(vtype, "");

	auto n1 = jive::test::structural_node_create(graph.root(), 3);
	auto i1 = n1->add_input(stype, s);
	auto i2 = n1->add_input(vtype, v);
	auto o1 = n1->add_output(stype);
	auto o2 = n1->add_output(vtype);

	auto a1 = n1->subregion(0)->add_argument(i1, stype);
	auto a2 = n1->subregion(0)->add_argument(i2, vtype);

	auto n2 = jive::test::simple_node_create(n1->subregion(0), {stype}, {a1}, {stype});
	auto n3 = jive::test::simple_node_create(n1->subregion(0), {vtype}, {a2}, {vtype});

	n1->subregion(0)->add_result(n2->output(0), o1, stype);
	n1->subregion(0)->add_result(n3->output(0), o2, vtype);

	jive::view(graph.root(), stdout);

	/* copy first into second region with arguments and results */
	jive::substitution_map smap;
	smap.insert(i1, i1); smap.insert(i2, i2);
	smap.insert(o1, o1); smap.insert(o2, o2);
	n1->subregion(0)->copy(n1->subregion(1), smap, true, true);

	jive::view(graph.root(), stdout);

	auto r2 = n1->subregion(1);
	assert(r2->narguments() == 2);
	assert(r2->argument(0)->input() == i1);
	assert(r2->argument(1)->input() == i2);

	assert(r2->nresults() == 2);
	assert(r2->result(0)->output() == o1);
	assert(r2->result(1)->output() == o2);

	assert(r2->nnodes() == 2);

	/* copy second into third region only with arguments */
	jive::substitution_map smap2;
	auto a3 = n1->subregion(2)->add_argument(i1, stype);
	auto a4 = n1->subregion(2)->add_argument(i2, vtype);
	smap2.insert(r2->argument(0), a3);
	smap2.insert(r2->argument(1), a4);

	smap2.insert(o1, o1); smap2.insert(o2, o2);
	n1->subregion(1)->copy(n1->subregion(2), smap2, false, true);

	jive::view(graph.root(), stdout);

	auto r3 = n1->subregion(2);
	assert(r3->nresults() == 2);
	assert(r3->result(0)->output() == o1);
	assert(r3->result(1)->output() == o2);

	assert(r3->nnodes() == 2);

	/* copy structural node */
	jive::substitution_map smap3;
	smap3.insert(s, s); smap3.insert(v, v);
	n1->copy(graph.root(), smap3);

	jive::view(graph.root(), stdout);

	assert(graph.root()->nnodes() == 2);
}

static inline void
test_node_depth()
{
	using namespace jive::test;

	valuetype vt;

	jive::graph graph;
	auto x = graph.add_import(vt, "x");

	auto null = simple_node_create(graph.root(), {}, {}, {vt});
	auto bin = simple_node_create(graph.root(), {vt, vt}, {null->output(0), x}, {vt});

	graph.add_export(bin->output(0), "x");

	jive::view(graph.root(), stdout);

	bin->input(0)->divert_origin(x);
	assert(bin->depth() == 0);

}

static int
test_nodes()
{
	test_node_copy();
	test_node_depth();

	return 0;
}

JIVE_UNIT_TEST_REGISTER("rvsdg/test-nodes", test_nodes);
