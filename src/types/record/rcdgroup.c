/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/record/rcdgroup.h>

#include <string.h>

#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>

static void
jive_group_node_init_(jive_group_node * self,
	struct jive_region * region, const jive_record_declaration * decl,
	size_t narguments, jive_output * const arguments[]);

static jive_node *
jive_group_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[]);

static void
jive_group_node_get_label_(const jive_node * self_, struct jive_buffer * buffer);

static bool
jive_group_node_match_attrs_(const jive_node * self, const jive_node_attrs * second);

static void
jive_group_node_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context);

const jive_node_class JIVE_GROUP_NODE = {
	parent : &JIVE_NODE,
	name : "GROUP",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : jive_group_node_get_label_, /* override */
	get_attrs : nullptr,
	match_attrs : jive_group_node_match_attrs_, /* override */
	check_operands : jive_group_node_check_operands_, /* override */
	create : jive_group_node_create_, /* override */
};

static void
jive_group_node_get_label_(const jive_node * self_, struct jive_buffer * buffer)
{
	jive_buffer_putstr(buffer, "GROUP");
}

static bool
jive_group_node_match_attrs_(const jive_node * self, const jive_node_attrs * second_)
{
	const jive::rcd::group_operation * first =
		(const jive::rcd::group_operation *)jive_node_get_attrs(self);
	const jive::rcd::group_operation * second = (const jive::rcd::group_operation *)second_;
	
	return first->declaration() == second->declaration();
}

static void
jive_group_node_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[], jive_context * context)
{
	const jive::rcd::group_operation * attrs = (const jive::rcd::group_operation *)attrs_;
	if (attrs->declaration()->nelements != noperands)
		jive_context_fatal_error(context,
			"Type mismatch: number of parameters to group does not match record declaration");

	size_t n;
	for (n = 0; n < noperands; n++) {
		const jive_type * type = attrs->declaration()->elements[n];
		if (!jive_type_equals(type, jive_output_get_type(operands[n])))
			jive_raise_type_error(type, jive_output_get_type(operands[n]), context);
	}
}

static jive_node *
jive_group_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	const jive::rcd::group_operation * attrs = (const jive::rcd::group_operation *)attrs_ ;

	jive_group_node * node = new jive_group_node(*attrs);

	if (attrs->declaration()->nelements != noperands) {
		jive_context_fatal_error(region->graph->context,
			"Type mismatch: number of parameters to group does not match record declaration");
	}

	node->class_ = &JIVE_GROUP_NODE;
	jive_group_node_init_(node, region, attrs->declaration(), noperands, operands);

	return node;
}

static void
jive_group_node_init_(jive_group_node * self,
	struct jive_region * region, const jive_record_declaration * decl,
	size_t narguments, jive_output * const arguments[])
{
	if (decl->nelements != narguments) {
		jive_context_fatal_error(region->graph->context,
			"Type mismatch: number of parameters to group does not match record declaration");
	}

	size_t n;
	const jive_type * arg_types[narguments];
	for(n = 0; n < narguments; n++) {
		arg_types[n] = decl->elements[n];
	}

	jive_record_type type(decl);
	const jive_type * rtype = &type ;

	jive_node_init_(self, region,
		narguments, arg_types, arguments,
		1, &rtype);

	type.class_->fini(&type);
}

jive_output *
jive_group_create(const jive_record_declaration * decl,
	size_t narguments, jive_output * const * arguments)
{
	jive::rcd::group_operation op(decl);
	jive_output * result;
	jive_graph * graph = arguments[0]->node->region->graph;
	jive_node_create_normalized(&JIVE_GROUP_NODE, graph, &op, narguments, arguments, &result);
	return result;
}

jive_output *
jive_empty_group_create(struct jive_graph * graph, const jive_record_declaration * decl)
{
	jive::rcd::group_operation op(decl);
	jive_output * result;
	jive_node_create_normalized(&JIVE_GROUP_NODE, graph, &op, 0, nullptr, &result);
	return result;
}
