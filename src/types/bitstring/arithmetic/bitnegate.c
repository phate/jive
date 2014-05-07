/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bitnegate.h>
#include <jive/types/bitstring/bitoperation-classes-private.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

static jive_node *
jive_bitnegate_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

static jive_unop_reduction_path_t
jive_bitnegate_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs,
	const jive_output * operand);

static jive_output *
jive_bitnegate_reduce_operand_(jive_unop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs, jive_output * operand);

const jive_bitunary_operation_class JIVE_BITNEGATE_NODE_ = {
	base : { /* jive_unary_operation_class */
		base : { /* jive_node_class */
			parent : &JIVE_BITUNARY_NODE,
			name : "BITNEGATE",
			fini : jive_node_fini_, /* inherit */
			get_default_normal_form : jive_unary_operation_get_default_normal_form_, /* inherit */
			get_label : jive_node_get_label_, /* inherit */
			get_attrs : nullptr,
			match_attrs : jive_node_match_attrs_, /* inherit */
			check_operands : jive_bitunary_operation_check_operands_, /* inherit */
			create : jive_bitnegate_create_, /* override */
		},
		
		single_apply_over : NULL,
		multi_apply_over : NULL,
		
		can_reduce_operand : jive_bitnegate_can_reduce_operand_, /* override */
		reduce_operand : jive_bitnegate_reduce_operand_ /* override */
	},
	type : jive_bitop_code_negate
};

static void
jive_bitnegate_node_init_(
	jive_node * self,
	jive_region * region,
	jive_output * operand)
{
	size_t nbits = jive_bitstring_output_nbits((jive_bitstring_output *)operand);

	jive_bitstring_type type(nbits);
	const jive_type * result_type[] = {&type};
	jive_node_init_(self, region,
		1, result_type, &operand,
		1, result_type);
}

static jive_node *
jive_bitnegate_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);
	
	jive_node * node = jive::create_operation_node(jive::bitstring::negate_operation());
	node->class_ = &JIVE_BITNEGATE_NODE;
	jive_bitnegate_node_init_(node, region, operands[0]);
	return node;
}

static jive_unop_reduction_path_t
jive_bitnegate_can_reduce_operand_(const jive_node_class * cls,
	const jive_node_attrs * attrs, const jive_output * operand)
{
	if (jive_node_isinstance(operand->node, &JIVE_BITNEGATE_NODE))
		return jive_unop_reduction_inverse;
	if (jive_node_isinstance(operand->node, &JIVE_BITCONSTANT_NODE))
		return jive_unop_reduction_constant;

	return jive_unop_reduction_none;
}

static jive_output *
jive_bitnegate_reduce_operand_(jive_unop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs_, jive_output * operand)
{
	if (path == jive_unop_reduction_inverse)
		return operand->node->inputs[0]->origin();

	if (path == jive_unop_reduction_constant) {
		const jive_bitconstant_node * node = (const jive_bitconstant_node *) operand->node;

		char bits[node->operation().bits.size()];
		jive_bitstring_negate(bits, &node->operation().bits[0], node->operation().bits.size());
		
		return jive_bitconstant(node->graph, node->operation().bits.size(), bits);
	}
	
	return NULL;
}

jive_output *
jive_bitnegate(jive_output * operand)
{
	jive::bitstring::negate_operation op;
	return jive_unary_operation_create_normalized(&JIVE_BITNEGATE_NODE_.base, operand->node->graph,
		&op, operand);
}
