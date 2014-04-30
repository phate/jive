/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <locale.h>
#include <assert.h>

#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/arch/address.h>
#include <jive/arch/address-transform.h>
#include <jive/arch/memlayout-simple.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/traverser.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	JIVE_DECLARE_ADDRESS_TYPE(addrtype);
	jive_bitstring_type bits8(8);
	jive_bitstring_type bits16(16);
	jive_bitstring_type bits32(32);
	const jive_value_type *  tmparray0[] = {&bits8, &bits16, &bits32, &bits32};
	
	jive_record_declaration decl = {
		nelements : 4,
		elements : tmparray0
	};
	const jive_value_type * tmparray1[] = {&bits8, &bits16, &bits32, &bits32};
	
	/*
	jive_record_declaration * decl = jive_record_declaration_create(context, 4,
		tmparray1);	
	*/

	const jive_type * tmparray10[] = {&bits32};
	jive_node * top = jive_node_create(graph->root_region,
		0, NULL, NULL,
		1, tmparray10);

	jive_output * address = jive_bitstring_to_address_create(top->outputs[0], 32, addrtype);

	jive_output * member0 = jive_memberof(address, &decl, 0);
	jive_output * member1 = jive_memberof(address, &decl, 1);
	jive_output * member2 = jive_memberof(address, &decl, 2);
	jive_output * member3 = jive_memberof(address, &decl, 3);

	jive_output * offset0 = jive_address_to_bitstring_create(member0, 32,
		jive_output_get_type(member0));
	jive_output * offset1 = jive_address_to_bitstring_create(member1, 32,
		jive_output_get_type(member1));
	jive_output * offset2 = jive_address_to_bitstring_create(member2, 32,
		jive_output_get_type(member2));
	jive_output * offset3 = jive_address_to_bitstring_create(member3, 32,
		jive_output_get_type(member3));
	const jive_type * tmparray2[] = {&bits32, &bits32, &bits32, &bits32};
jive_output * tmparray3[] = {offset0, offset1, offset2, offset3};

	const jive_type * tmparray11[] = {&bits8};
	jive_node * bottom = jive_node_create(graph->root_region,
		4, tmparray2,
		tmparray3,
		1, tmparray11);
	jive_graph_export(graph, bottom->outputs[0]);

	jive_view(graph, stdout);

	jive_memlayout_mapper_simple mapper;
	jive_memlayout_mapper_simple_init(&mapper, context, 32);
	jive_graph_address_transform(graph, &mapper.base.base);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);
	jive_view(graph, stdout);

	jive_traverser * traverser = jive_topdown_traverser_create(graph);
	jive_node * node = jive_traverser_next(traverser);
	for(; node; node = jive_traverser_next(traverser)){
		size_t i;
		for(i = 0; i < node->ninputs; i++){
			assert(!jive_type_isinstance(jive_input_get_type(node->inputs[i]), &JIVE_ADDRESS_TYPE));	
		}
		for(i = 0; i < node->noutputs; i++){
			assert(!jive_type_isinstance(jive_output_get_type(node->outputs[i]), &JIVE_ADDRESS_TYPE));	
		}
	}
	jive_traverser_destroy(traverser);

	jive_node * sum = bottom->inputs[0]->origin->node;
	assert(jive_node_isinstance(sum, &JIVE_BITSUM_NODE));
	jive_node * constant = sum->inputs[1]->origin->node;
	assert(jive_node_isinstance(constant, &JIVE_BITCONSTANT_NODE));
	assert(jive_bitconstant_equals_unsigned(jive_bitconstant_node_cast(constant), 0));
	
	sum = bottom->inputs[1]->origin->node;
	assert(jive_node_isinstance(sum, &JIVE_BITSUM_NODE));
	constant = sum->inputs[1]->origin->node;
	assert(jive_node_isinstance(constant, &JIVE_BITCONSTANT_NODE));
	assert(jive_bitconstant_equals_unsigned(jive_bitconstant_node_cast(constant), 2));

	sum = bottom->inputs[2]->origin->node;
	assert(jive_node_isinstance(sum, &JIVE_BITSUM_NODE));
	constant = sum->inputs[1]->origin->node;
	assert(jive_node_isinstance(constant, &JIVE_BITCONSTANT_NODE));
	assert(jive_bitconstant_equals_unsigned(jive_bitconstant_node_cast(constant), 4));

	sum = bottom->inputs[3]->origin->node;
	assert(jive_node_isinstance(sum, &JIVE_BITSUM_NODE));
	constant = sum->inputs[1]->origin->node;
	assert(jive_node_isinstance(constant, &JIVE_BITCONSTANT_NODE));
	assert(jive_bitconstant_equals_unsigned(jive_bitconstant_node_cast(constant), 8));

	jive_memlayout_mapper_simple_fini(&mapper);
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/address-transform/test-memberof-transform", test_main);
