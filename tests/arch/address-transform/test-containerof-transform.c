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
#include <jive/vsdg/traverser.h>
#include <jive/vsdg/node-private.h>

#include "testnodes.h"

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	jive_address_type addrtype;
	jive::bits::type bits8(8);
	jive::bits::type bits16(16);
	jive::bits::type bits32(32);
	const jive_value_type * tmparray0[] = {&bits8, &bits16, &bits32, &bits32};
	
	jive_record_declaration decl = {
		nelements : 4,
		elements : tmparray0
	};
	const jive_type * tmparray1[] = {&bits32, &bits32, &bits32, &bits32};
	
	jive_node * top = jive_test_node_create(graph->root_region,
		0, NULL, NULL,
		4, tmparray1);

	jive_output * address0 = jive_bitstring_to_address_create(top->outputs[0], 32, &addrtype);
	jive_output * address1 = jive_bitstring_to_address_create(top->outputs[1], 32, &addrtype);
	jive_output * address2 = jive_bitstring_to_address_create(top->outputs[2], 32, &addrtype);
	jive_output * address3 = jive_bitstring_to_address_create(top->outputs[3], 32, &addrtype);
	
	jive_output * container0 = jive_containerof(address0, &decl, 0);
	jive_output * container1 = jive_containerof(address1, &decl, 1);
	jive_output * container2 = jive_containerof(address2, &decl, 2);
	jive_output * container3 = jive_containerof(address3, &decl, 3);

	jive_output * offset0 = jive_address_to_bitstring_create(container0, 32, &container0->type());
	jive_output * offset1 = jive_address_to_bitstring_create(container1, 32, &container1->type());
	jive_output * offset2 = jive_address_to_bitstring_create(container2, 32, &container2->type());
	jive_output * offset3 = jive_address_to_bitstring_create(container3, 32, &container3->type());
	const jive_type * tmparray2[] = {&bits32, &bits32, &bits32, &bits32};
	jive_output * tmparray3[] = {offset0, offset1, offset2, offset3};

	const jive_type * tmparray11[] = {&bits8};
	jive_node * bottom = jive_test_node_create(graph->root_region,
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
			assert(!dynamic_cast<const jive_address_type*>(&node->inputs[i]->type()));
		}
		for(i = 0; i < node->noutputs; i++){
			assert(!dynamic_cast<const jive_address_type*>(&node->outputs[i]->type()));
		}
	}
	jive_traverser_destroy(traverser);
	
	jive_node * sum = bottom->producer(0);
	assert(jive_node_isinstance(sum, &JIVE_BITDIFFERENCE_NODE));
	jive_node * constant = sum->producer(1);
	assert(jive_node_isinstance(constant, &JIVE_BITCONSTANT_NODE));
	assert(jive_bitconstant_equals_signed(dynamic_cast<jive_bitconstant_node *>(constant), 0));
	
	sum = bottom->producer(1);
	assert(jive_node_isinstance(sum, &JIVE_BITDIFFERENCE_NODE));
	constant = sum->producer(1);
	assert(jive_node_isinstance(constant, &JIVE_BITCONSTANT_NODE));
	assert(jive_bitconstant_equals_signed(dynamic_cast<jive_bitconstant_node *>(constant), 2));

	sum = bottom->producer(2);
	assert(jive_node_isinstance(sum, &JIVE_BITDIFFERENCE_NODE));
	constant = sum->producer(1);
	assert(jive_node_isinstance(constant, &JIVE_BITCONSTANT_NODE));
	assert(jive_bitconstant_equals_signed(dynamic_cast<jive_bitconstant_node *>(constant), 4));

	sum = bottom->producer(3);
	assert(jive_node_isinstance(sum, &JIVE_BITDIFFERENCE_NODE));
	constant = sum->producer(1);
	assert(jive_node_isinstance(constant, &JIVE_BITCONSTANT_NODE));
	assert(jive_bitconstant_equals_signed(dynamic_cast<jive_bitconstant_node *>(constant), 8));
	
	jive_memlayout_mapper_simple_fini(&mapper);
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);	

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/address-transform/test-containerof-transform", test_main);
