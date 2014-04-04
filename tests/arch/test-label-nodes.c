/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <string.h>

#include <jive/arch/address.h>
#include <jive/arch/address-transform.h>
#include <jive/arch/linker-symbol.h>
#include <jive/context.h>
#include <jive/view.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/node-private.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);
	
	jive_label_external foobar, bla;
	
	jive_linker_symbol foobar_symbol;
	jive_linker_symbol bla_symbol;
	jive_label_external_init(&foobar, context, "foobar", &foobar_symbol);
	jive_label_external_init(&bla, context, "bla", &bla_symbol);

	jive_output * o0 = jive_label_to_address_create(graph, &foobar.base);
	jive_output * o1 = jive_label_to_address_create(graph, &bla.base);

	const jive_label_to_address_node_attrs * attrs0 = (const jive_label_to_address_node_attrs *)
		jive_node_get_attrs(o0->node);
	const jive_label_to_address_node_attrs * attrs1 = (const jive_label_to_address_node_attrs *)
		jive_node_get_attrs(o1->node);

	assert(attrs0);
	assert(attrs1);
	assert(attrs0->label == &foobar.base);
	assert(attrs1->label == &bla.base);
	
	assert(!jive_node_match_attrs(o0->node, attrs1));
	assert(jive_node_match_attrs(o1->node, attrs1));
	
	jive_output * o2 = jive_label_to_bitstring_create(graph, &foobar.base, 32);
	jive_output * o3 = jive_label_to_bitstring_create(graph, &bla.base, 32);	
	jive_output * o4 = jive_label_to_bitstring_create(graph, &foobar.base, 16);

	const jive_label_to_bitstring_node_attrs * attrs2 = (const jive_label_to_bitstring_node_attrs *)
		jive_node_get_attrs(o2->node);
	const jive_label_to_bitstring_node_attrs * attrs3 = (const jive_label_to_bitstring_node_attrs *)
		jive_node_get_attrs(o3->node);
	const jive_label_to_bitstring_node_attrs * attrs4 = (const jive_label_to_bitstring_node_attrs *)
		jive_node_get_attrs(o4->node);

	assert(attrs2);
	assert(attrs3);
	assert(attrs4);
	assert(attrs2->label == &foobar.base);
	assert(attrs3->label == &bla.base);
	assert(attrs4->label == &foobar.base);
	
	assert(!jive_node_match_attrs(o2->node, attrs4));
	assert(!jive_node_match_attrs(o2->node, attrs3));
	assert(jive_node_match_attrs(o2->node, attrs2));
	
	JIVE_DECLARE_ADDRESS_TYPE(addr);
	JIVE_DECLARE_BITSTRING_TYPE(bits32, 32);
	JIVE_DECLARE_BITSTRING_TYPE(bits16, 16);
	const jive_type * tmparray0[] = {addr, addr, bits32, bits32, bits16};
	jive_output * tmparray1[] = {o0, o1, o2, o3, o4};
	jive_node * bottom = jive_node_create(graph->root_region,
		5, tmparray0,
		tmparray1,
		1, &addr);
	jive_graph_export(graph, bottom->outputs[0]);

	jive_view(graph, stderr);

	jive_label_to_address_node_address_transform(jive_label_to_address_node_cast(o0->node), 32);
	jive_label_to_address_node_address_transform(jive_label_to_address_node_cast(o1->node), 32);

	jive_graph_prune(graph);
	jive_view(graph, stderr);

	jive_graph_destroy(graph);
	
	jive_label_external_fini(&bla);
	jive_label_external_fini(&foobar);

	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}


JIVE_UNIT_TEST_REGISTER("arch/test-label-nodes", test_main);
