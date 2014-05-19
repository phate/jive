/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/float/comparison/fltequal.h>
#include <jive/types/float/fltoperation-classes-private.h>

#include <jive/types/float/flttype.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>

static jive_node *
jive_fltequal_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

const jive_fltcomparison_operation_class JIVE_FLTEQUAL_NODE_ = {
	base : {	/* jive_binary_operation_class */
		base : {	/* jive_node_class */
			parent : &JIVE_FLTCOMPARISON_NODE,
			name : "FLTREQUAL",
			fini : jive_node_fini_, /* inherit */
			get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
			get_label : jive_node_get_label_, /* inherit */
			match_attrs : jive_node_match_attrs_, /* inherit */
			check_operands : jive_fltcomparison_operation_check_operands_, /* inherit */
			create : jive_fltequal_node_create_, /* override */
		},
		flags : jive_binary_operation_commutative,
		single_apply_under : NULL,
		multi_apply_under : NULL,
		distributive_over : NULL,
		distributive_under : NULL,

		can_reduce_operand_pair : jive_binary_operation_can_reduce_operand_pair_, /* inherit */
		reduce_operand_pair : jive_binary_operation_reduce_operand_pair_ /* inherit */
	},
	type : jive_fltcmp_code_equal
};

static void
jive_fltequal_node_init_(jive_node * self, jive_region * region,
	jive_output * op1, jive_output * op2)
{
	jive_control_type ctype;
	const jive_type * ctype_ptr = &ctype;
	jive_float_type flttype;
	const jive_type * tmparray0[] = {&flttype, &flttype};
	jive_output * tmparray1[] = {op1, op2};
	jive_node_init_(self, region,
		2, tmparray0, tmparray1,
		1, &ctype_ptr);
}

static jive_node *
jive_fltequal_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 2);

	jive_node * node = jive::create_operation_node(jive::flt::equal_operation());
	node->class_ = &JIVE_FLTEQUAL_NODE;
	jive_fltequal_node_init_(node, region, operands[0], operands[1]);

	return node;
}

jive_output *
jive_fltequal(struct jive_output * op1, struct jive_output * op2)
{
	jive_graph * graph = op1->node()->graph;
	jive_output * tmparray2[] = {op1, op2};
	jive::flt::equal_operation op;
	return jive_binary_operation_create_normalized(&JIVE_FLTEQUAL_NODE_.base, graph, &op, 2,
		tmparray2);
}
