/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/float/arithmetic/fltnegate.h>
#include <jive/types/float/fltoperation-classes-private.h>

#include <jive/types/float/flttype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>

static jive_node *
jive_fltnegate_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

const jive_fltunary_operation_class JIVE_FLTNEGATE_NODE_ = {
	base : { /* jive_unary_opeartion_class */
		base : {	/* jive_node_class */
			parent : &JIVE_FLTUNARY_NODE,
			name : "FLTNEGATE",
			fini : jive_node_fini_, /* inherit */
			get_default_normal_form : jive_unary_operation_get_default_normal_form_, /* inherit */
			get_label : jive_node_get_label_, /* inherit */
			match_attrs : jive_node_match_attrs_, /* inherit */
			check_operands : jive_fltbinary_operation_check_operands_, /* inherit */
			create : jive_fltnegate_node_create_, /* overrride */
		},
		
		single_apply_over : NULL,
		multi_apply_over : NULL,

		can_reduce_operand : jive_unary_operation_can_reduce_operand_, /* inherit */
		reduce_operand : jive_unary_operation_reduce_operand_ /* inherit */
	},
	type : jive_fltop_code_negate
};

static void
jive_fltnegate_node_init_(struct jive_node * self, struct jive_region * region,
	struct jive_output * operand)
{
	jive::flt::type flttype;
	const jive::base::type * flttype_ptr = &flttype;
	jive_node_init_(self, region,
		1, &flttype_ptr, &operand,
		1, &flttype_ptr);
}

static jive_node *
jive_fltnegate_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);

	jive_node * node = jive::create_operation_node(jive::flt::negate_operation());
	node->class_ = &JIVE_FLTNEGATE_NODE;
	jive_fltnegate_node_init_(node, region, operands[0]);
	return node;
}

jive_output *
jive_fltnegate(struct jive_output * operand)
{
	jive::flt::negate_operation op;
	return jive_unary_operation_create_normalized(&JIVE_FLTNEGATE_NODE_.base, operand->node()->graph,
		&op, operand);
}
