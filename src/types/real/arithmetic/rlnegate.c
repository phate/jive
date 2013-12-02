/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/real/arithmetic/rlnegate.h>
#include <jive/types/real/rloperation-classes-private.h>
#include <jive/types/real/rltype.h>
#include <jive/vsdg/node-private.h>

static jive_node *
jive_rlnegate_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

const jive_rlunary_operation_class JIVE_RLNEGATE_NODE_ = {
	.base = { /* jive_unary_opeartion_class */
		.base = {	/* jive_node_class */
			.parent = &JIVE_RLUNARY_NODE,
			.name = "RLNEGATE",
			.fini = jive_node_fini_, /* inherit */
			.get_default_normal_form = jive_unary_operation_get_default_normal_form_, /* inherit */
			.get_label = jive_node_get_label_, /* inherit */
			.get_attrs = jive_node_get_attrs_, /* inherit */
			.match_attrs = jive_node_match_attrs_, /* inherit */
			.check_operands = jive_rlunary_operation_check_operands_, /* override */
			.create = jive_rlnegate_node_create_, /* overrride */
			.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
		},

		.single_apply_over = NULL,
		.multi_apply_over = NULL,

		.can_reduce_operand = jive_unary_operation_can_reduce_operand_, /* inherit */
		.reduce_operand = jive_unary_operation_reduce_operand_ /* inherit */
	},

	.type = jive_rlop_code_negate
};

static void
jive_rlnegate_node_init_(struct jive_rlnegate_node * self, struct jive_region * region,
	struct jive_output * operand)
{
	JIVE_DECLARE_REAL_TYPE(rltype);
	jive_node_init_(&self->base, region,
		1, &rltype, &operand,
		1, &rltype);
}

static jive_node *
jive_rlnegate_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);

	jive_rlnegate_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->base.class_ = &JIVE_RLNEGATE_NODE;
	jive_rlnegate_node_init_(node, region, operands[0]);
	return &node->base;
}

struct jive_output *
jive_rlnegate(struct jive_output * operand)
{
	return jive_unary_operation_create_normalized(&JIVE_RLNEGATE_NODE_.base, operand->node->graph,
		NULL, operand);
}
