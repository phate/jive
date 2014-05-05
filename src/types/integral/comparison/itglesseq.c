/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/integral/comparison/itglesseq.h>
#include <jive/types/integral/itgoperation-classes-private.h>
#include <jive/types/integral/itgtype.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>

static jive_node *
jive_itglesseq_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

const jive_itgcomparison_operation_class JIVE_ITGLESSEQ_NODE_ = {
	base : { /* jive_binary_operation_class */
		base : { /* jive_node_class */
			parent : &JIVE_ITGCOMPARISON_NODE,
			name : "ITGLESSEQ",
			fini : jive_node_fini_, /* inherit */
			get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
			get_label : jive_node_get_label_, /* inherit */
			get_attrs : jive_node_get_attrs_, /* inherit */
			match_attrs : jive_node_match_attrs_, /* inherit */
			check_operands : jive_itgcomparison_operation_check_operands_, /* inherit */
			create : jive_itglesseq_node_create_, /* override */
		},

		flags : jive_binary_operation_none,
		single_apply_under : NULL,
		multi_apply_under : NULL,
		distributive_over : NULL,
		distributive_under : NULL,

		can_reduce_operand_pair : jive_binary_operation_can_reduce_operand_pair_, /* inherit */
		reduce_operand_pair : jive_binary_operation_reduce_operand_pair_ /* inherit */
	},

	type : jive_itgcmp_code_lesseq,
};

static void
jive_itglesseq_node_init_(jive_itglesseq_node * self, jive_region * region,
	jive_output * operand1, jive_output * operand2)
{
	jive_control_type ctype;
	const jive_type * ctype_ptr = &ctype;
	jive_integral_type itgtype;
	const jive_type * tmparray0[] = {&itgtype, &itgtype};
	jive_output * tmparray1[] = {operand1, operand2};
	jive_node_init_(self, region,
		2, tmparray0, tmparray1,
		1, &ctype_ptr);
}

static jive_node *
jive_itglesseq_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 2);

	jive_itglesseq_node * node = new jive_itglesseq_node;
	node->class_ = &JIVE_ITGLESSEQ_NODE;
	jive_itglesseq_node_init_(node, region, operands[0], operands[1]);

	return node;
}

struct jive_output *
jive_itglesseq(struct jive_output * operand1, struct jive_output * operand2)
{
	jive_graph * graph = operand1->node->graph;
	jive_output * tmparray2[] = {operand1, operand2};
	return jive_binary_operation_create_normalized(&JIVE_ITGLESSEQ_NODE_.base, graph, NULL, 2,
		tmparray2);
}
