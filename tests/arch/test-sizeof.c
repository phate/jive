/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/view.h>
#include <jive/types/bitstring.h>
#include <jive/arch/addresstype.h>
#include <jive/types/record/rcdtype.h>
#include <jive/types/union/unntype.h>
#include <jive/arch/sizeof.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/traverser.h>
#include <jive/arch/memlayout-simple.h>

#include "testnodes.h"

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	jive::bits::type bits4(4);
	jive::bits::type bits8(8);
	jive::bits::type bits18(18);
	jive::bits::type bits32(32);
	jive_address_type addr;
	const jive_value_type *  tmparray0[] = {&bits4, &bits8, &bits18};

	jive_record_declaration r_decl = {3, tmparray0};
	
	jive_record_type record_t(&r_decl);
	const jive_value_type *  tmparray1[] = {&bits4, &bits8, &bits18};

	jive_union_declaration u_decl = {3, tmparray1};

	jive_union_type union_t(&u_decl);

	jive_output * s0 = jive_sizeof_create(graph->root_region, &bits4);
	jive_output * s1 = jive_sizeof_create(graph->root_region, &bits8);
	jive_output * s2 = jive_sizeof_create(graph->root_region, &bits8);
	jive_output * s3 = jive_sizeof_create(graph->root_region, &bits18);
	jive_output * s4 = jive_sizeof_create(graph->root_region, &bits32);
	jive_output * s5 = jive_sizeof_create(graph->root_region, &addr);
	jive_output * s6 = jive_sizeof_create(graph->root_region, &record_t);
	jive_output * s7 = jive_sizeof_create(graph->root_region, &union_t);

	assert(jive_node_match_attrs(s1->node(), jive_node_get_attrs(s2->node())));
	const jive_type *  tmparray2[] = {&bits32, &bits32, &bits32, &bits32, &bits32, &bits32, &bits32,
		&bits32};
	jive_output *  tmparray3[] = {s0, s1, s2, s3, s4, s5, s6, s7};
	assert(!jive_node_match_attrs(s0->node(), jive_node_get_attrs(s3->node())));

	jive_node * bottom = jive_test_node_create(graph->root_region,
		8, tmparray2,
		tmparray3,
		1, tmparray2);
	jive_graph_export(graph, bottom->outputs[0]);

	jive_view(graph, stdout);

	jive_memlayout_mapper_simple layout_mapper;
	jive_memlayout_mapper_simple_init(&layout_mapper, context, 32);	
	jive_traverser * traverser = jive_topdown_traverser_create(graph);
	jive_node * node;
	for (node = jive_traverser_next(traverser); node; node = jive_traverser_next(traverser)) {
		if (jive_node_isinstance(node, &JIVE_SIZEOF_NODE))
			jive_sizeof_node_reduce(jive_sizeof_node_cast(node), &layout_mapper.base.base);				
	}
	jive_traverser_destroy(traverser);
	jive_graph_prune(graph);

	assert(jive_bitconstant_equals_unsigned(dynamic_cast<jive_bitconstant_node *>(
		bottom->producer(0)), 1));
	assert(jive_bitconstant_equals_unsigned(dynamic_cast<jive_bitconstant_node *>(
		bottom->producer(1)), 1));
	assert(jive_bitconstant_equals_unsigned(dynamic_cast<jive_bitconstant_node *>(
		bottom->producer(2)), 1));
	assert(jive_bitconstant_equals_unsigned(dynamic_cast<jive_bitconstant_node *>(
		bottom->producer(3)), 4));
	assert(jive_bitconstant_equals_unsigned(dynamic_cast<jive_bitconstant_node *>(
		bottom->producer(4)), 4));
	assert(jive_bitconstant_equals_unsigned(dynamic_cast<jive_bitconstant_node *>(
		bottom->producer(5)), 4));
	assert(jive_bitconstant_equals_unsigned(dynamic_cast<jive_bitconstant_node *>(
		bottom->producer(6)), 8));
	assert(jive_bitconstant_equals_unsigned(dynamic_cast<jive_bitconstant_node *>(
		bottom->producer(7)), 4));
	
	jive_view(graph, stdout);

	jive_memlayout_mapper_simple_fini(&layout_mapper);

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;	
}

JIVE_UNIT_TEST_REGISTER("arch/test-sizeof", test_main);
