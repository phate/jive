/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/arch/addresstype.h>
#include <jive/arch/memorytype.h>
#include <jive/arch/store.h>
#include <jive/types/bitstring/type.h>
#include <jive/types/record.h>
#include <jive/types/union.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>

#include "testnodes.h"

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();
	
	static const jive::bits::type bits8(8);
	static const jive::bits::type bits16(16);
	static const jive::bits::type bits32(32);

	static const jive::value::type * decl_elems[] = {&bits8, &bits16, &bits32};
	std::shared_ptr<const jive::rcd::declaration> rcddecl(
		new jive::rcd::declaration({&bits8, &bits16, &bits32}));
	static jive::rcd::type rcdtype(rcddecl);
	
	static const jive::unn::declaration unndecl = {3, decl_elems};
	static jive::unn::type unntype(&unndecl);

	static const jive::unn::declaration empty_unndecl = {0, NULL};
	static jive::unn::type empty_unntype(&empty_unndecl);

	jive::mem::type memtype;
	jive::addr::type addrtype;
	jive_node * top = jive_test_node_create(graph->root_region, {}, {},
		{&addrtype, &memtype, &bits8, &bits16, &bits32, &memtype, &addrtype});

	jive::output * state = top->output(1);
	std::vector<jive::output *> states0 = jive_store_by_address_create(
		top->output(0), &bits32, top->output(4), 1, &state);

	jive::output * tmparray1[] = {top->output(2), top->output(3), top->output(4)};
	jive::output * group = jive_group_create(rcddecl, 3, tmparray1);
	jive::output * tmparray2[] = {top->output(1), top->output(5)};
	std::vector<jive::output *> states1 = jive_store_by_address_create(
		top->output(0), &rcdtype, group, 2, tmparray2);

	jive::output * unify = jive_unify_create(&unndecl, 2, top->output(4));
	std::vector<jive::output *> states2 = jive_store_by_address_create(
		top->output(0), &unntype, unify, 1, &state);

	std::vector<jive::output *> states3 = jive_store_by_address_create(
		top->output(6), &bits32, top->output(4), 1, &state);
	std::vector<jive::output *> states4 = jive_store_by_address_create(
		top->output(0), &bits32, top->output(4), 1, &states3[0]);

	unify = jive_empty_unify_create(graph->root_region, &empty_unndecl);
	std::vector<jive::output *> states5 = jive_store_by_address_create(
		top->output(0), &empty_unntype, unify, 1, &state);

	jive_node * bottom = jive_test_node_create(graph->root_region,
		std::vector<const jive::base::type*>(6, &memtype),
		{states0[0], states1[0], states1[0], states2[0], states4[0], states5[0]},
		{&memtype});
	jive_graph_export(graph, bottom->output(0));

	jive_graph_normalize(graph);
	jive_graph_prune(graph);

	jive_view(graph, stderr);

	assert(states3[0]->node()->input(2)->origin()->node() == top);

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-store", test_main);
