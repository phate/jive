/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/union/unnunify.h>

#include <jive/types/union/unntype.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/operators/nullary.h>

#include <string.h>

/* unify node */

static void
jive_unify_node_init_(jive_unify_node * self,
	struct jive_region * region, const jive_union_declaration * decl,
	size_t option, jive_output * const operand);

static jive_node *
jive_unify_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[]);

static void
jive_unify_node_get_label_(const jive_node * self_, struct jive_buffer * buffer);

static bool
jive_unify_node_match_attrs_(const jive_node * self, const jive_node_attrs * second);

static void
jive_unify_node_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context);

const jive_unary_operation_class JIVE_UNIFY_NODE_ = {
	base : { /* jive_node_class */
		parent : &JIVE_UNARY_OPERATION,
		name : "UNIFY",
		fini : jive_node_fini_, /* inherit */
		get_default_normal_form : jive_unary_operation_get_default_normal_form_, /* inherit */
		get_label : jive_unify_node_get_label_, /* override */
		get_attrs : nullptr,
		match_attrs : jive_unify_node_match_attrs_, /* override */
		check_operands : jive_unify_node_check_operands_, /* override */
		create : jive_unify_node_create_, /* override */
	},

	single_apply_over : NULL,
	multi_apply_over : NULL,

	can_reduce_operand : jive_unary_operation_can_reduce_operand_, /* inherit */
	reduce_operand : jive_unary_operation_reduce_operand_ /* inherit */
};

static void
jive_unify_node_get_label_(const jive_node * self_, struct jive_buffer * buffer)
{
	jive_buffer_putstr(buffer, "UNIFY");
}

static bool
jive_unify_node_match_attrs_(const jive_node * self, const jive_node_attrs * second_)
{
	const jive::unn::unify_operation * first = &((const jive_unify_node *)self)->operation();
	const jive::unn::unify_operation * second = (const jive::unn::unify_operation *)second_;
	
	return
		first->declaration() == second->declaration() &&
		first->option() == second->option();
}

static void
jive_unify_node_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[], jive_context * context)
{
	JIVE_DEBUG_ASSERT(noperands == 1);

	const jive::unn::unify_operation * attrs = (const jive::unn::unify_operation *)attrs_;

	if (attrs->option() >= attrs->declaration()->nelements)
		jive_context_fatal_error(context, "Type mismatch: invalid option for union type");

	const jive_type * type = attrs->declaration()->elements[attrs->option()];
	if (!jive_type_equals(type, jive_output_get_type(operands[0])))
		jive_raise_type_error(type, jive_output_get_type(operands[0]), context);
}

static jive_node *
jive_unify_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	const jive::unn::unify_operation * attrs = (const jive::unn::unify_operation *)attrs_ ;

	jive_unify_node * node = new jive_unify_node(*attrs);
	node->class_ = &JIVE_UNIFY_NODE;
	jive_unify_node_init_(node, region, attrs->declaration(), attrs->option(), operands[0]);

	return node;
}

static void
jive_unify_node_init_(jive_unify_node * self,
	struct jive_region * region, const jive_union_declaration * decl,
	size_t option, jive_output * const operand)
{
	if (option >= decl->nelements) {
		jive_context_fatal_error(region->graph->context,
			"Type mismatch: invalid option for union type");
	}
	
	const jive_type * arg_type = decl->elements[option];
	
	jive_union_type type(decl);
	const jive_type * type_ptr = &type;
	jive_node_init_(self, region,
		1, &arg_type, &operand,
		1, &type_ptr);
}

jive_output *
jive_unify_create(const jive_union_declaration * decl,
	size_t option, jive_output * const operand)
{
	jive::unn::unify_operation op(decl, option);

	return jive_unary_operation_create_normalized(&JIVE_UNIFY_NODE_, operand->node->graph,
		&op, operand);
}

/* empty unify node */

static jive_node *
jive_empty_unify_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[]);

static bool
jive_empty_unify_node_match_attrs_(const jive_node * self, const jive_node_attrs * second);

const jive_node_class JIVE_EMPTY_UNIFY_NODE = {
	parent : &JIVE_NULLARY_OPERATION,
	name : "UNIFY",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_nullary_operation_get_default_normal_form_, /* inherit */
	get_label : jive_node_get_label_, /* inherit */
	get_attrs : nullptr,
	match_attrs : jive_empty_unify_node_match_attrs_, /* override */
	check_operands : jive_node_check_operands_, /* inherit */
	create : jive_empty_unify_node_create_, /* override */
};

static void
jive_empty_unify_node_init_(jive_empty_unify_node * self,
	struct jive_region * region, const jive_union_declaration * decl)
{
	jive_union_type type(decl);
	const jive_type * type_ptr = &type;
	jive_node_init_(self, region,
		0, NULL, NULL,
		1, &type_ptr);
}

static bool
jive_empty_unify_node_match_attrs_(const jive_node * self, const jive_node_attrs * second_)
{
	const jive::unn::empty_unify_operation * first =
		&((const jive_empty_unify_node *) self)->operation();
	const jive::unn::empty_unify_operation * second =
		(const jive::unn::empty_unify_operation *) second_;
	
	return (first->declaration() == second->declaration());
}

static jive_node *
jive_empty_unify_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 0);
	const jive::unn::empty_unify_operation * attrs = (const jive::unn::empty_unify_operation *) attrs_;
	
	jive_empty_unify_node * node = new jive_empty_unify_node(*attrs);
	node->class_ = &JIVE_EMPTY_UNIFY_NODE;
	jive_empty_unify_node_init_(node, region, attrs->declaration());
	
	return node;
}

jive_output *
jive_empty_unify_create(struct jive_graph * graph, const jive_union_declaration * decl)
{
	jive::unn::empty_unify_operation op(decl);

	return jive_nullary_operation_create_normalized(&JIVE_EMPTY_UNIFY_NODE, graph, &op);
}
