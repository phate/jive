/*
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bitnot.h>

#include <jive/vsdg/region.h>
#include <jive/vsdg/node-private.h>
#include <jive/types/bitstring/constant.h>
#include <jive/types/bitstring/bitstring-operations.h>

static jive_node *
jive_bitnot_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

static jive_unop_reduction_path_t
jive_bitnot_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs,
	const jive_output * operand);

static jive_output *
jive_bitnot_reduce_operand_(jive_unop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs, jive_output * operand);

const jive_bitunary_operation_class JIVE_BITNOT_NODE_ = {
	.base = { /* jive_unary_operation_class */
		.base = { /* jive_node_class */
			.parent = &JIVE_BITUNARY_NODE,
			.name = "BITNOT",
			.fini = jive_node_fini_, /* inherit */
			.get_default_normal_form = jive_unary_operation_get_default_normal_form_, /* inherit */
			.get_label = jive_node_get_label_, /* inherit */
			.get_attrs = jive_node_get_attrs_, /* inherit */
			.match_attrs = jive_node_match_attrs_, /* inherit */
			.create = jive_bitnot_create_, /* override */
			.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
		},
		
		.single_apply_over = NULL,
		.multi_apply_over = NULL,
		
		.can_reduce_operand = jive_bitnot_can_reduce_operand_, /* override */
		.reduce_operand = jive_bitnot_reduce_operand_ /* override */
	},
	.type = jive_bitop_code_not
};

static void
jive_bitnot_node_init_(
	jive_node * self,
	jive_region * region,
	jive_output * operand)
{
	if (!jive_output_isinstance(operand, &JIVE_BITSTRING_OUTPUT)){
		jive_context_fatal_error(region->graph->context, "Type mismatch: bitnot node requires bitstring operands");
	}
	size_t nbits = ((jive_bitstring_output *) operand)->type.nbits;
	
	JIVE_DECLARE_BITSTRING_TYPE(type, nbits);
	jive_node_init_(self, region,
		1, &type, &operand,
		1, &type);
}

static jive_node *
jive_bitnot_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);
	
	jive_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->class_ = &JIVE_BITNOT_NODE;
	jive_bitnot_node_init_(node, region, operands[0]);
	return node;
}

static jive_unop_reduction_path_t
jive_bitnot_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs_,
	const jive_output * operand_)
{
	if (operand_->node->class_ == &JIVE_BITNOT_NODE)
		return jive_unop_reduction_inverse;
	if (operand_->node->class_ == &JIVE_BITCONSTANT_NODE)
		return jive_unop_reduction_constant;
	
	return 0;
}

static jive_output *
jive_bitnot_reduce_operand_(jive_unop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs_, jive_output * operand_)
{
	if (path == jive_unop_reduction_inverse) {
		return operand_->node->inputs[0]->origin;
	}
	
	if (path == jive_unop_reduction_constant) {
		const jive_bitconstant_node * node = (const jive_bitconstant_node *) operand_->node;
		char bits[node->attrs.nbits];
		jive_bitstring_not(bits, node->attrs.bits, node->attrs.nbits);
		
		return jive_bitconstant(node->base.graph, node->attrs.nbits, bits);
	}
	
	return NULL;
}

jive_node *
jive_bitnot_create(struct jive_region * region, jive_output * operand)
{
	const jive_unary_operation_normal_form * nf =
		(const jive_unary_operation_normal_form *)
		jive_graph_get_nodeclass_form(operand->node->graph, &JIVE_BITNOT_NODE);

	return jive_unary_operation_normalized_create(nf, region, NULL, operand)->node;
}

jive_output *
jive_bitnot(jive_output * operand)
{
	const jive_unary_operation_normal_form * nf =
		(const jive_unary_operation_normal_form *)
		jive_graph_get_nodeclass_form(operand->node->graph, &JIVE_BITNOT_NODE);
	
	return jive_unary_operation_normalized_create(nf, operand->node->region, NULL,
		operand);
}
