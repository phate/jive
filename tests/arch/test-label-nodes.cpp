/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.hpp"

#include <assert.h>
#include <string.h>

#include <jive/arch/address-transform.hpp>
#include <jive/arch/address.hpp>
#include <jive/arch/linker-symbol.hpp>
#include <jive/arch/memlayout-simple.hpp>
#include <jive/rvsdg/graph.hpp>
#include <jive/rvsdg/label.hpp>
#include <jive/view.hpp>

#include "testnodes.hpp"

static int test_main(void)
{
	using namespace jive;

	jive::graph graph;

	jive_linker_symbol bla_symbol;
	jive_linker_symbol foobar_symbol;
	jive::external_label bla("bla", &bla_symbol);
	jive::external_label foobar("foobar", &foobar_symbol);

	auto o0 = lbl2addr_op::create(graph.root(), &foobar);
	auto o1 = lbl2addr_op::create(graph.root(), &bla);

	auto attrs0 = dynamic_cast<const lbl2addr_op*>(&node_output::node(o0)->operation());
	auto attrs1 = dynamic_cast<const lbl2addr_op*>(&node_output::node(o1)->operation());

	assert(attrs0->label() == &foobar);
	assert(attrs1->label() == &bla);
	
	assert(node_output::node(o0)->operation() != *attrs1);
	
	auto o2 = lbl2bit_op::create(graph.root(), 32, &foobar);
	auto o3 = lbl2bit_op::create(graph.root(), 32, &bla);
	auto o4 = lbl2bit_op::create(graph.root(), 16, &foobar);

	auto attrs2 = dynamic_cast<const lbl2bit_op*>(&node_output::node(o2)->operation());
	auto attrs3 = dynamic_cast<const lbl2bit_op*>(&node_output::node(o3)->operation());
	auto attrs4 = dynamic_cast<const lbl2bit_op*>(&node_output::node(o4)->operation());

	assert(attrs2->label() == &foobar);
	assert(attrs3->label() == &bla);
	assert(attrs4->label() == &foobar);
	
	assert(node_output::node(o2)->operation() != *attrs4);
	assert(node_output::node(o2)->operation() != *attrs3);
	
	graph.add_export(o0, {o0->type(), ""});
	graph.add_export(o1, {o1->type(), ""});
	graph.add_export(o2, {o2->type(), ""});
	graph.add_export(o3, {o3->type(), ""});
	graph.add_export(o4, {o4->type(), ""});

	jive::view(graph.root(), stderr);

	memlayout_mapper_simple mapper(4);
	transform_address(node_output::node(o0), mapper);
	transform_address(node_output::node(o1), mapper);

	graph.prune();
	jive::view(graph.root(), stderr);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-label-nodes", test_main)
