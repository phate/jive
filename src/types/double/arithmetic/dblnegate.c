/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/double/arithmetic/dblnegate.h>
#include <jive/types/double/dbloperation-classes-private.h>
#include <jive/types/double/dbltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>

static jive_node *
jive_dblnegate_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

const jive_dblunary_operation_class JIVE_DBLNEGATE_NODE_ = {
	.base = { /* jive_unary_opeartion_class */
		.base = {	/* jive_node_class */
			.parent = &JIVE_DBLUNARY_NODE,
			.name = "DBLNEGATE",
			.fini = jive_node_fini_, /* inherit */
			.get_default_normal_form = jive_unary_operation_get_default_normal_form_, /* inherit */
			.get_label = jive_node_get_label_, /* inherit */
			.get_attrs = jive_node_get_attrs_, /* inherit */
			.match_attrs = jive_node_match_attrs_, /* inherit */
			.check_operands = jive_dblunary_operation_check_operands_, /* inherit */
			.create = jive_dblnegate_node_create_, /* overrride */
			.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
		},

		.single_apply_over = NULL,
		.multi_apply_over = NULL,

		.can_reduce_operand = jive_unary_operation_can_reduce_operand_, /* inherit */
		.reduce_operand = jive_unary_operation_reduce_operand_ /* inherit */
	},
	.type = jive_dblop_code_negate
};

static void
jive_dblnegate_node_init_(struct jive_node * self, struct jive_region * region,
	struct jive_output * operand)
{
	JIVE_DECLARE_DOUBLE_TYPE(dbltype);
	jive_node_init_(self, region,
		1, &dbltype, &operand,
		1, &dbltype);
}

static jive_node *
jive_dblnegate_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);

	jive_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->class_ = &JIVE_DBLNEGATE_NODE;
	jive_dblnegate_node_init_(node, region, operands[0]);
	return node;
}

struct jive_output *
jive_dblnegate(struct jive_output * operand)
{
	return jive_unary_operation_create_normalized(&JIVE_DBLNEGATE_NODE_.base, operand->node->graph,
		NULL, operand);
}
