/*
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/float/comparison/fltgreater.h>
#include <jive/types/float/fltoperation-classes-private.h>

#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node-private.h>
#include <jive/types/float/flttype.h>

static jive_node *
jive_fltgreater_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

const jive_fltcomparison_operation_class JIVE_FLTGREATER_NODE_ = {
	.base = { /* jive_binary_operation_class */
		.base = { /* jive_node_class */
			.parent = &JIVE_FLTCOMPARISON_NODE,
			.name = "FLTGREATER",
			.fini = jive_node_fini_, /* inherit */
			.get_default_normal_form = jive_binary_operation_get_default_normal_form_, /* inherit */
			.get_label = jive_node_get_label_, /* inherit */
			.get_attrs = jive_node_get_attrs_, /* inherit */
			.match_attrs = jive_node_match_attrs_, /* inherit */
			.check_operands = jive_fltcomparison_operation_check_operands_, /* inherit */
			.create = jive_fltgreater_node_create_, /* override */
			.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
		},
		.flags = jive_binary_operation_none,
		.single_apply_under = NULL,
		.multi_apply_under = NULL,
		.distributive_over = NULL,
		.distributive_under = NULL,

		.can_reduce_operand_pair = jive_binary_operation_can_reduce_operand_pair_, /* inherit */
		.reduce_operand_pair = jive_binary_operation_reduce_operand_pair_ /* inherit */
	},
	.type = jive_fltcmp_code_greater
};

static void
jive_fltgreater_node_init_(struct jive_node * self, struct jive_region * region,
	struct jive_output * op1, struct jive_output * op2)
{
	JIVE_DECLARE_CONTROL_TYPE(ctype);
	JIVE_DECLARE_FLOAT_TYPE(flttype);
	jive_node_init_(self, region,
		2, (const jive_type *[]){flttype, flttype}, (jive_output *[]){op1, op2},
		1, &ctype);
}

static jive_node *
jive_fltgreater_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 2);
	
	jive_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->class_ = &JIVE_FLTGREATER_NODE;
	jive_fltgreater_node_init_(node, region, operands[0], operands[1]);

	return node;
}

jive_output *
jive_fltgreater(jive_output * op1, jive_output * op2)
{
	jive_graph * graph = op1->node->graph;
	return jive_binary_operation_create_normalized(&JIVE_FLTGREATER_NODE_.base, graph, NULL, 2,
		(jive_output *[]){op1, op2});
}
