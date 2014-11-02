/*
 * Copyright 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/arch/addresstype.h>
#include <jive/arch/load.h>
#include <jive/context.h>
#include <jive/types/bitstring.h>
#include <jive/types/union.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>

#include "testnodes.h"

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create();

	static const jive::bits::type bits8(8);
	static const jive::bits::type bits16(16);
	static const jive::bits::type bits32(32);

	static const jive::value::type * decl_elems[] = {&bits8, &bits16, &bits32};
	static const jive::unn::declaration decl = {3, decl_elems};
	static jive::unn::type unntype(&decl);

	jive::addr::type addrtype;
	const jive::base::type * tmparray0[] = {&bits8, &unntype, &unntype, &addrtype};
	jive_node * top = jive_test_node_create(graph->root_region,
		0, NULL, NULL,
		4, tmparray0);

	jive::output * u0 = jive_unify_create(&decl, 0, top->outputs[0]);
	jive::output * load = jive_load_by_address_create(top->outputs[3], &unntype, 0, NULL);

	jive::output * c0 = jive_choose_create(1, top->outputs[1]);
	jive::output * c1 = jive_choose_create(0, u0);
	jive::output * c2 = jive_choose_create(1, top->outputs[2]);
	jive::output * c3 = jive_choose_create(0, load);
	const jive::base::type * tmparray1[] = {&bits16, &bits8, &bits16, &bits8};
	jive::output * tmparray2[] = {c0, c1, c2, c3};

	jive_node * bottom = jive_test_node_create(graph->root_region,
		4, tmparray1, tmparray2,
		1, tmparray0);
	jive_graph_export(graph, bottom->outputs[0]);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);

	jive_view(graph, stderr);

	assert(bottom->producer(1) == top);
	assert(jive_node_match_attrs(c0->node(), jive_node_get_attrs(c2->node())));
	assert(dynamic_cast<const jive::load_op *>(&bottom->producer(3)->operation()));

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/union/test-unnchoose", test_main);
