/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/integral/arithmetic/itgproduct.h>
#include <jive/types/integral/itgoperation-classes-private.h>
#include <jive/types/integral/itgtype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>

static jive_node *
jive_itgproduct_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

const jive_itgbinary_operation_class JIVE_ITGPRODUCT_NODE_ = {
	base : { /* jive_binary_operation_class */
		base : { /* jive_node_class */
			parent : &JIVE_ITGBINARY_NODE,
			name : "ITGPRODUCT",
			fini : jive_node_fini_, /* inherit */
			get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
			get_label : jive_node_get_label_, /* inherit */
			get_attrs : jive_node_get_attrs_, /* inherit */
			match_attrs : jive_node_match_attrs_, /* inherit */
			check_operands : jive_itgbinary_operation_check_operands_, /* inherit */
			create : jive_itgproduct_node_create_, /* override */
			get_aux_rescls : jive_node_get_aux_rescls_ /* inherit */
		},

		flags : jive_binary_operation_associative | jive_binary_operation_commutative,
		single_apply_under : NULL,
		multi_apply_under : NULL,
		distributive_over : NULL,
		distributive_under : NULL,

		can_reduce_operand_pair : jive_binary_operation_can_reduce_operand_pair_, /* inherit */
		reduce_operand_pair : jive_binary_operation_reduce_operand_pair_ /* inherit */

	},

	type : jive_itgop_code_product
};

static void
jive_itgproduct_node_init_(jive_itgproduct_node * self, jive_region * region,
	size_t noperands, jive_output * const operands[])
{
	size_t n;
	const jive_type * operand_types[noperands];
	JIVE_DECLARE_INTEGRAL_TYPE(itgtype);
	for (n = 0; n < noperands; n++)
		operand_types[n] = itgtype;

	jive_node_init_(&self->base, region,
		noperands, operand_types, operands,
		1, &itgtype);
}

static jive_node *
jive_itgproduct_node_create_(jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[])
{
	jive_itgproduct_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->base.class_ = &JIVE_ITGPRODUCT_NODE;
	jive_itgproduct_node_init_(node, region, noperands, operands);

	return &node->base;
}

struct jive_output *
jive_itgproduct(struct jive_output * operand1, struct jive_output * operand2)
{
	jive_graph * graph = operand1->node->graph;
	jive_output * tmparray0[] = {operand1, operand2};
	return jive_binary_operation_create_normalized(&JIVE_ITGPRODUCT_NODE_.base, graph, NULL, 2,
		tmparray0);
}
