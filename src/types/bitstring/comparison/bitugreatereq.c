/*
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/comparison/bitugreatereq.h>

#include <jive/vsdg/region.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node-private.h>
#include <jive/types/bitstring/bitoperation-classes-private.h>
#include <jive/types/bitstring/constant.h>

static jive_node *
jive_bitugreatereq_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

static jive_binop_reduction_path_t
jive_bitugreatereq_node_can_reduce_operand_pair_(const jive_node_class * cls,
	const jive_node_attrs * attrs, const jive_output * op1, const jive_output * op2);

static jive_output *
jive_bitugreatereq_node_reduce_operand_pair_(jive_binop_reduction_path_t path,
	const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * op1,
	jive_output * op2);

const jive_bitcomparison_operation_class JIVE_BITUGREATEREQ_NODE_ = {
	base : { /* jive_binary_operation_class */
		base : { /* jive_node_class */
			parent : &JIVE_BITCOMPARISON_NODE,
			name : "BITUGREATEREQ",
			fini : jive_node_fini_, /* inherit */
			get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
			get_label : jive_node_get_label_, /* inherit */
			get_attrs : jive_node_get_attrs_, /* inherit */
			match_attrs : jive_node_match_attrs_, /* inherit */
			check_operands : jive_bitcomparison_operation_check_operands_, /* inherit */
			create : jive_bitugreatereq_create_, /* override */
		},
		flags : jive_binary_operation_none,
		single_apply_under : NULL,
		multi_apply_under : NULL,
		distributive_over : NULL,
		distributive_under : NULL,
	
		can_reduce_operand_pair : jive_bitugreatereq_node_can_reduce_operand_pair_, /* override */
		reduce_operand_pair : jive_bitugreatereq_node_reduce_operand_pair_ /* override */
	},
	type : jive_bitcmp_code_ugreatereq,
	compare_constants : NULL
};

static void
jive_bitugreatereq_node_init_(jive_node * self, jive_region * region,
	jive_output * operand1, jive_output * operand2)
{
	size_t nbits = jive_bitstring_output_nbits((jive_bitstring_output *)operand1);

	JIVE_DECLARE_CONTROL_TYPE(ctype);
	JIVE_DECLARE_BITSTRING_TYPE(btype, nbits);
	const jive_type * tmparray0[] = {btype, btype};
	jive_output * tmparray1[] = {operand1, operand2};
	jive_node_init_(self, region,
		2, tmparray0, tmparray1,
		1, &ctype);
}

static jive_node *
jive_bitugreatereq_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 2);

	jive_node * node = new jive_node;
	node->class_ = &JIVE_BITUGREATEREQ_NODE;
	jive_bitugreatereq_node_init_(node, region, operands[0], operands[1]);

	return node;
}

static jive_binop_reduction_path_t 
jive_bitugreatereq_node_can_reduce_operand_pair_(const jive_node_class * cls,
	const jive_node_attrs * attrs, const jive_output * op1, const jive_output * op2)
{
	const jive_bitconstant_node * n1 = jive_bitconstant_node_cast(op1->node);
	const jive_bitconstant_node * n2 = jive_bitconstant_node_cast(op2->node);

	/* constant < constant */
	if (n1 && n2){
		JIVE_DEBUG_ASSERT(n1->attrs.nbits == n2->attrs.nbits);
		char result = jive_bitstring_ugreatereq(n1->attrs.bits, n2->attrs.bits,
			n1->attrs.nbits);

		switch(result){
			case '0': return 1;
			case '1': return 2;
			default: return jive_binop_reduction_none;
		}
	}

	/* constant >= INT_MIN */
	if (n2){
		size_t nbits = n2->attrs.nbits;
		char int_min[nbits];
		jive_bitstring_init_signed(int_min, nbits, 0);
	
		if (jive_bitstring_equal(n2->attrs.bits, int_min, nbits) == '1')
			return 3;
	}
	
	/* INT_MAX >= constant */
	if (n1){
		size_t nbits = n1->attrs.nbits;
		char int_max[nbits];
		jive_bitstring_init_signed(int_max, nbits, -1);

		if (jive_bitstring_equal(n1->attrs.bits, int_max, nbits) == '1')
			return 4;
	}

	return jive_binop_reduction_none;
}

static jive_output *
jive_bitugreatereq_node_reduce_operand_pair_(jive_binop_reduction_path_t path,
	const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * op1,
	jive_output * op2)
{
	jive_graph * graph = op1->node->graph;

	switch(path){
		case 2:
		case 3:
		case 4:
			return jive_control_true(graph);
		case 1:
			return jive_control_false(graph);
		default:
			return NULL;
	}
}

jive_output *
jive_bitugreatereq(jive_output * operand1, jive_output * operand2)
{
	jive_graph * graph = operand1->node->graph;
	jive_output * tmparray2[] = {operand1, operand2};
	return jive_binary_operation_create_normalized(&JIVE_BITUGREATEREQ_NODE_.base, graph, NULL, 2,
		tmparray2);
}
