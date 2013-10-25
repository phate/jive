/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/integral/arithmetic/itgnegate.h>
#include <jive/types/integral/itgoperation-classes-private.h>
#include <jive/types/integral/itgtype.h>
#include <jive/vsdg/node-private.h>

static jive_node *
jive_itgnegate_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

const jive_itgunary_operation_class JIVE_ITGNEGATE_NODE_ = {
	.base = { /* jive_unary_operation_class */
		.base = { /* jive_node_class */
			.parent = &JIVE_ITGUNARY_NODE,
			.name = "ITGNEGATE",
			.fini = jive_node_fini_, /* inherit */
			.get_default_normal_form = jive_unary_operation_get_default_normal_form_, /* inherit */
			.get_label = jive_node_get_label_, /* inherit */
			.get_attrs = jive_node_get_attrs_, /* inherit */
			.match_attrs = jive_node_match_attrs_, /* inherit */
			.check_operands = jive_itgunary_operation_check_operands_, /* inherit */
			.create = jive_itgnegate_node_create_, /* override */
			.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
		},

		.single_apply_over = NULL,
		.multi_apply_over = NULL,

		.can_reduce_operand = jive_unary_operation_can_reduce_operand_, /* inherit */
		.reduce_operand = jive_unary_operation_reduce_operand_ /* inherit */
	},

	.type = jive_itgop_code_negate
};

static void
jive_itgnegate_node_init_(jive_itgnegate_node * self, jive_region * region, jive_output * operand)
{
	JIVE_DECLARE_INTEGRAL_TYPE(itgtype);
	jive_node_init_(&self->base, region,
		1, &itgtype, &operand,
		1, &itgtype);
}

static jive_node *
jive_itgnegate_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);

	jive_itgnegate_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->base.class_ = &JIVE_ITGNEGATE_NODE;
	jive_itgnegate_node_init_(node, region, operands[0]);
	return &node->base;
}

struct jive_output *
jive_itgnegate(struct jive_output * operand)
{
	return jive_unary_operation_create_normalized(&JIVE_ITGNEGATE_NODE_.base, operand->node->graph,
		NULL, operand);
}
