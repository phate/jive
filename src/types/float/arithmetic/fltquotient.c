/*
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/float/arithmetic/fltquotient.h>

#include <jive/vsdg/node-private.h>
#include <jive/types/float/flttype.h>

static jive_node *
jive_fltquotient_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

const jive_fltbinary_operation_class JIVE_FLTQUOTIENT_NODE_ = {
	.base = {	/* jive_fltbinary_operation_class */
		.base = {
			.parent = &JIVE_FLTBINARY_NODE,
			.name = "FLTQUOTIENT",
			.fini = jive_node_fini_, /* inherit */
			.get_default_normal_form = jive_binary_operation_get_default_normal_form_, /* inherit */
			.get_label = jive_node_get_label_, /* inherit */
			.get_attrs = jive_node_get_attrs_, /* inherit */
			.match_attrs = jive_node_match_attrs_, /* inherit */
			.create = jive_fltquotient_node_create_, /* override */
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
	.type = jive_fltop_code_quotient
};

static void
jive_fltquotient_node_init_(jive_node * self, jive_region * region,
	struct jive_output * op1, struct jive_output * op2)
{
	JIVE_DECLARE_FLOAT_TYPE(flttype);
	jive_node_init_(self, region,
		2, (const jive_type *[]){flttype, flttype}, (jive_output*[]){op1, op2},
		1, &flttype);
}

static jive_node *
jive_fltquotient_node_create_(jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 2);

	jive_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->class_ = &JIVE_FLTQUOTIENT_NODE;
	jive_fltquotient_node_init_(node, region, operands[0], operands[1]);

	return node;
}

jive_output *
jive_fltquotient(struct jive_output * op1, struct jive_output * op2)
{
	jive_region * region = jive_region_innermost(2, (jive_output *[]){op1, op2});
	return jive_binary_operation_normalized_create(&JIVE_FLTQUOTIENT_NODE, region, NULL,
		2, (jive_output *[]){op1, op2});
}

