/*
 * Copyright 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.hpp"
#include "testnodes.hpp"

#include <assert.h>

#include <jive/arch/addresstype.hpp>
#include <jive/arch/load.hpp>
#include <jive/rvsdg.hpp>
#include <jive/types/bitstring.hpp>
#include <jive/types/union.hpp>
#include <jive/view.hpp>


static int test_unnchoose(void)
{
	using namespace jive;

	jive::graph graph;

	auto dcl = unndeclaration::create(&graph, {&bit8, &bit16, &bit32});
	jive::unntype unntype(dcl);

	auto i0 = graph.add_import({bit8, ""});
	auto i1 = graph.add_import({unntype, ""});
	auto i2 = graph.add_import({unntype, ""});

	auto u0 = jive_unify_create(dcl, 0, i0);

	auto c0 = choose_op::create(i1, 1);
	auto c1 = choose_op::create(u0, 0);
	auto c2 = choose_op::create(i2, 1);

	graph.add_export(c0, {c0->type(), ""});
	auto x1 = graph.add_export(c1, {c1->type(), ""});
	graph.add_export(c2, {c2->type(), ""});

	graph.normalize();
	graph.prune();

	jive::view(graph.root(), stderr);

	assert(x1->origin() == i0);
	assert(c0->node()->operation() == c2->node()->operation());

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/union/test-unnchoose", test_unnchoose)

static int test_unnunify(void)
{
	using namespace jive;

	jive::graph graph;
	
	auto dcl = unndeclaration::create(&graph, {&bit8, &bit16, &bit32});
	jive::unntype unntype(dcl);

	auto edcl = unndeclaration::create(&graph);
	jive::unntype unntype_empty(edcl);

	auto i0 = graph.add_import({bit8, ""});

	auto u0 = jive_unify_create(dcl, 0, i0);
	auto u1 = jive_empty_unify_create(graph.root(), edcl);

	graph.add_export(u0, {u0->type(), ""});
	graph.add_export(u1, {u1->type(), ""});

	graph.normalize();
	graph.prune();

	jive::view(graph.root(), stderr);

	assert(u0->node()->operation() != u1->node()->operation());

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/union/test-unnunify", test_unnunify)
