/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
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

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);
	
	static const jive::bits::type bits8(8);
	static const jive::bits::type bits16(16);
	static const jive::bits::type bits32(32);

	static const jive::value::type * decl_elems[] = {&bits8, &bits16, &bits32};
	static const jive::rcd::declaration rcddecl = {3, decl_elems};
	static jive::rcd::type rcdtype(&rcddecl);
	
	static const jive::unn::declaration unndecl = {3, decl_elems};
	static jive::unn::type unntype(&unndecl);

	static const jive::unn::declaration empty_unndecl = {0, NULL};
	static jive::unn::type empty_unntype(&empty_unndecl);

	jive::mem::type memtype;
	jive::addr::type addrtype;
	const jive::base::type * tmparray0[] = {
		&addrtype, &memtype, &bits8, &bits16, &bits32, &memtype, &addrtype
	};
	jive_node * top = jive_test_node_create(graph->root_region,
		0, NULL, NULL,
		7, tmparray0);

	std::vector<jive::output *> states0 = jive_store_by_address_create(
		top->outputs[0], &bits32, top->outputs[4], 1, &top->outputs[1]);

	jive::output * tmparray1[] = {top->outputs[2],
		top->outputs[3], top->outputs[4]};
	jive::output * group = jive_group_create(&rcddecl, 3, tmparray1);
	jive::output * tmparray2[] = {top->outputs[1], top->outputs[5]};
	std::vector<jive::output *> states1 = jive_store_by_address_create(
		top->outputs[0], &rcdtype, group, 2, tmparray2);

	jive::output * unify = jive_unify_create(&unndecl, 2, top->outputs[4]);
	std::vector<jive::output *> states2 = jive_store_by_address_create(
		top->outputs[0], &unntype, unify, 1, &top->outputs[1]);

	std::vector<jive::output *> states3 = jive_store_by_address_create(
		top->outputs[6], &bits32, top->outputs[4], 1, &top->outputs[1]);
	std::vector<jive::output *> states4 = jive_store_by_address_create(
		top->outputs[0], &bits32, top->outputs[4], 1, &states3[0]);

	unify = jive_empty_unify_create(graph, &empty_unndecl);
	std::vector<jive::output *> states5 = jive_store_by_address_create(
		top->outputs[0], &empty_unntype, unify, 1, &top->outputs[1]);
	const jive::base::type * tmparray3[] = {
		&memtype, &memtype, &memtype, &memtype, &memtype, &memtype
	};
	jive::output * tmparray4[] = {
		states0[0], states1[0], states1[1], states2[0], states4[0], states5[0]
	};

	jive_node * bottom = jive_test_node_create(graph->root_region,
		6, tmparray3,
		tmparray4,
		1, tmparray3);
	jive_graph_export(graph, bottom->outputs[0]);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);

	jive_view(graph, stderr);

	assert(states3[0]->node()->producer(2) == top);

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-store", test_main);
