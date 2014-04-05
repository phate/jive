/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/double/arithmetic/dblproduct.h>
#include <jive/types/double/dbloperation-classes-private.h>
#include <jive/types/double/dbltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>

static jive_node *
jive_dblproduct_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

const jive_dblbinary_operation_class JIVE_DBLPRODUCT_NODE_ = {
	base : {	/* jive_dblbinary_operation_class */
		base : { /* jive_node_class */
			parent : &JIVE_DBLBINARY_NODE,
			name : "DBLPRODUCT",
			fini : jive_node_fini_, /* inherit */
			get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
			get_label : jive_node_get_label_, /* inherit */
			get_attrs : jive_node_get_attrs_, /* inherit */
			match_attrs : jive_node_match_attrs_, /* inherit */
			check_operands : jive_dblbinary_operation_check_operands_, /* inherit */
			create : jive_dblproduct_node_create_, /* overrride */
			get_aux_rescls : jive_node_get_aux_rescls_ /* inherit */
		},

		flags : jive_binary_operation_commutative,
		single_apply_under : NULL,
		multi_apply_under : NULL,
		distributive_over : NULL,
		distributive_under : NULL,

		can_reduce_operand_pair : jive_binary_operation_can_reduce_operand_pair_, /* inherit */
		reduce_operand_pair : jive_binary_operation_reduce_operand_pair_ /* inherit */
	},

	type : jive_dblop_code_product
};

static void
jive_dblproduct_node_init_(jive_node * self, jive_region * region,
	struct jive_output * op1, struct jive_output * op2)
{
	JIVE_DECLARE_DOUBLE_TYPE(dbltype);
	const jive_type * tmparray0[] = {dbltype, dbltype};
	jive_output * tmparray1[] = {op1, op2};
	jive_node_init_(self, region,
		2, tmparray0, tmparray1,
		1, &dbltype);
}

static jive_node *
jive_dblproduct_node_create_(jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 2);

	jive_node * node = new jive_node;
	node->class_ = &JIVE_DBLPRODUCT_NODE;
	jive_dblproduct_node_init_(node, region, operands[0], operands[1]);

	return node;
}

jive_output *
jive_dblproduct(struct jive_output * op1, struct jive_output  * op2)
{
	jive_graph * graph = op1->node->graph;
	jive_output * tmparray2[] = {op1, op2};
	return jive_binary_operation_create_normalized(&JIVE_DBLPRODUCT_NODE_.base, graph, NULL, 2,
		tmparray2);
}
