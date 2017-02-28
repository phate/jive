/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>

#include <jive/arch/addresstype.h>
#include <jive/arch/memlayout-simple.h>
#include <jive/arch/sizeof.h>
#include <jive/types/bitstring.h>
#include <jive/types/record/rcdtype.h>
#include <jive/types/union/unntype.h>
#include <jive/view.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/simple_node.h>
#include <jive/vsdg/traverser.h>

#include "testnodes.h"

static int test_main(void)
{
	jive::graph graph;

	jive::bits::type bits4(4);
	jive::bits::type bits8(8);
	jive::bits::type bits18(18);
	jive::bits::type bits32(32);
	jive::addr::type addr;
	std::shared_ptr<const jive::rcd::declaration> r_decl(
		new jive::rcd::declaration({&bits4, &bits8, &bits18}));

	jive::rcd::type record_t(r_decl);
	const jive::value::type *  tmparray1[] = {&bits4, &bits8, &bits18};

	jive::unn::declaration u_decl = {3, tmparray1};

	jive::unn::type union_t(&u_decl);

	auto s0 = dynamic_cast<jive::output*>(jive_sizeof_create(graph.root(), &bits4));
	auto s1 = dynamic_cast<jive::output*>(jive_sizeof_create(graph.root(), &bits8));
	auto s2 = dynamic_cast<jive::output*>(jive_sizeof_create(graph.root(), &bits8));
	auto s3 = dynamic_cast<jive::output*>(jive_sizeof_create(graph.root(), &bits18));
	auto s4 = jive_sizeof_create(graph.root(), &bits32);
	auto s5 = jive_sizeof_create(graph.root(), &addr);
	auto s6 = jive_sizeof_create(graph.root(), &record_t);
	auto s7 = jive_sizeof_create(graph.root(), &union_t);

	assert(s1->node()->operation() == s2->node()->operation());
	assert(s0->node()->operation() != s3->node()->operation());

	jive::node * bottom = jive_test_node_create(graph.root(),
		std::vector<const jive::base::type*>(8, &bits32), {s0, s1, s2, s3, s4, s5, s6, s7}, {&bits32});
	graph.export_port(bottom->output(0), "dummy");

	jive::view(graph.root(), stdout);

	jive::memlayout_mapper_simple layout_mapper(4);
	for (jive::node * node : jive::topdown_traverser(graph.root())) {
		if (dynamic_cast<const jive::sizeof_op *>(&node->operation())) {
			jive_sizeof_node_reduce(node, &layout_mapper);
		}
	}
	graph.prune();

	assert(dynamic_cast<jive::output*>(bottom->input(0)->origin())->node()->operation()
		== jive::bits::uint_constant_op(32, 1));
	assert(dynamic_cast<jive::output*>(bottom->input(1)->origin())->node()->operation()
		== jive::bits::uint_constant_op(32, 1));
	assert(dynamic_cast<jive::output*>(bottom->input(2)->origin())->node()->operation()
		== jive::bits::uint_constant_op(32, 1));
	assert(dynamic_cast<jive::output*>(bottom->input(3)->origin())->node()->operation()
		== jive::bits::uint_constant_op(32, 4));
	assert(dynamic_cast<jive::output*>(bottom->input(4)->origin())->node()->operation()
		== jive::bits::uint_constant_op(32, 4));
	assert(dynamic_cast<jive::output*>(bottom->input(5)->origin())->node()->operation()
		== jive::bits::uint_constant_op(32, 4));
	assert(dynamic_cast<jive::output*>(bottom->input(6)->origin())->node()->operation()
		== jive::bits::uint_constant_op(32, 8));
	assert(dynamic_cast<jive::output*>(bottom->input(7)->origin())->node()->operation()
		== jive::bits::uint_constant_op(32, 4));
	
	jive::view(graph.root(), stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-sizeof", test_main);
