#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/view.h>
#include <jive/types/bitstring.h>
#include <jive/arch/addresstype.h>
#include <jive/types/record/rcdtype.h>
#include <jive/types/union/unntype.h>
#include <jive/arch/sizeof.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/traverser.h>
#include <jive/arch/memlayout-simple.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	JIVE_DECLARE_BITSTRING_TYPE(bits4, 4);
	JIVE_DECLARE_BITSTRING_TYPE(bits8, 8);
	JIVE_DECLARE_BITSTRING_TYPE(bits18, 18);
	JIVE_DECLARE_BITSTRING_TYPE(bits32, 32);
	JIVE_DECLARE_ADDRESS_TYPE(addr);

	jive_record_declaration r_decl = {3, (const jive_value_type * []){
		(const jive_value_type *)bits4,
		(const jive_value_type *)bits8,
		(const jive_value_type *)bits18}};
	
	jive_record_type record_t;
	jive_record_type_init(&record_t, &r_decl);	

	jive_union_declaration u_decl = {3, (const jive_value_type * []){
		(const jive_value_type *)bits4,
		(const jive_value_type *)bits8,
		(const jive_value_type *)bits18}};

	jive_union_type union_t;
	jive_union_type_init(&union_t, &u_decl);

	jive_output * s0 = jive_sizeof_create(graph->root_region, (const jive_value_type *)bits4);
	jive_output * s1 = jive_sizeof_create(graph->root_region, (const jive_value_type *)bits8);
	jive_output * s2 = jive_sizeof_create(graph->root_region, (const jive_value_type *)bits8);
	jive_output * s3 = jive_sizeof_create(graph->root_region, (const jive_value_type *)bits18);
	jive_output * s4 = jive_sizeof_create(graph->root_region, (const jive_value_type *)bits32);
	jive_output * s5 = jive_sizeof_create(graph->root_region, (const jive_value_type *)addr);
	jive_output * s6 = jive_sizeof_create(graph->root_region, &record_t.base);
	jive_output * s7 = jive_sizeof_create(graph->root_region, &union_t.base);

	assert(jive_node_match_attrs(s1->node, jive_node_get_attrs(s2->node)));
	assert(!jive_node_match_attrs(s0->node, jive_node_get_attrs(s3->node)));	

	jive_node * bottom = jive_node_create(graph->root_region,
		8, (const jive_type * []){bits32, bits32, bits32, bits32, bits32, bits32, bits32, bits32},
		(jive_output * []){s0, s1, s2, s3, s4, s5, s6, s7},
		0, NULL);
	jive_node_reserve(bottom);

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

	assert(jive_bitconstant_equals_unsigned(jive_bitconstant_node_cast(
		bottom->inputs[0]->origin->node), 1));
	assert(jive_bitconstant_equals_unsigned(jive_bitconstant_node_cast(
		bottom->inputs[1]->origin->node), 1));
	assert(jive_bitconstant_equals_unsigned(jive_bitconstant_node_cast(
		bottom->inputs[2]->origin->node), 1));
	assert(jive_bitconstant_equals_unsigned(jive_bitconstant_node_cast(
		bottom->inputs[3]->origin->node), 4));
	assert(jive_bitconstant_equals_unsigned(jive_bitconstant_node_cast(
		bottom->inputs[4]->origin->node), 4));
	assert(jive_bitconstant_equals_unsigned(jive_bitconstant_node_cast(
		bottom->inputs[5]->origin->node), 4)); 
	assert(jive_bitconstant_equals_unsigned(jive_bitconstant_node_cast(
		bottom->inputs[6]->origin->node), 8));
	assert(jive_bitconstant_equals_unsigned(jive_bitconstant_node_cast(
		bottom->inputs[7]->origin->node), 4));
	
	jive_view(graph, stdout);

	jive_memlayout_mapper_simple_fini(&layout_mapper);

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;	
}

JIVE_UNIT_TEST_REGISTER("arch/test-sizeof", test_main);
