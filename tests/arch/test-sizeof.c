/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/arch/addresstype.h>
#include <jive/arch/memlayout-simple.h>
#include <jive/arch/sizeof.h>
#include <jive/types/bitstring.h>
#include <jive/types/record/rcdtype.h>
#include <jive/types/union/unntype.h>
#include <jive/view.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/traverser.h>

#include "testnodes.h"

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

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

	jive::output * s0 = jive_sizeof_create(graph->root_region, &bits4);
	jive::output * s1 = jive_sizeof_create(graph->root_region, &bits8);
	jive::output * s2 = jive_sizeof_create(graph->root_region, &bits8);
	jive::output * s3 = jive_sizeof_create(graph->root_region, &bits18);
	jive::output * s4 = jive_sizeof_create(graph->root_region, &bits32);
	jive::output * s5 = jive_sizeof_create(graph->root_region, &addr);
	jive::output * s6 = jive_sizeof_create(graph->root_region, &record_t);
	jive::output * s7 = jive_sizeof_create(graph->root_region, &union_t);

	assert(s1->node()->operation() == s2->node()->operation());
	assert(s0->node()->operation() != s3->node()->operation());

	jive_node * bottom = jive_test_node_create(graph->root_region,
		std::vector<const jive::base::type*>(8, &bits32), {s0, s1, s2, s3, s4, s5, s6, s7}, {&bits32});
	jive_graph_export(graph, bottom->outputs[0]);

	jive_view(graph, stdout);

	jive_memlayout_mapper_simple layout_mapper;
	jive_memlayout_mapper_simple_init(&layout_mapper, 32);
	for (jive_node * node : jive::topdown_traverser(graph)) {
		if (dynamic_cast<const jive::sizeof_op *>(&node->operation())) {
			jive_sizeof_node_reduce(node, &layout_mapper.base.base);
		}
	}
	jive_graph_prune(graph);

	assert(bottom->producer(0)->operation() == jive::bits::uint_constant_op(32, 1));
	assert(bottom->producer(1)->operation() == jive::bits::uint_constant_op(32, 1));
	assert(bottom->producer(2)->operation() == jive::bits::uint_constant_op(32, 1));
	assert(bottom->producer(3)->operation() == jive::bits::uint_constant_op(32, 4));
	assert(bottom->producer(4)->operation() == jive::bits::uint_constant_op(32, 4));
	assert(bottom->producer(5)->operation() == jive::bits::uint_constant_op(32, 4));
	assert(bottom->producer(6)->operation() == jive::bits::uint_constant_op(32, 8));
	assert(bottom->producer(7)->operation() == jive::bits::uint_constant_op(32, 4));
	
	jive_view(graph, stdout);

	jive_memlayout_mapper_simple_fini(&layout_mapper);

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-sizeof", test_main);
