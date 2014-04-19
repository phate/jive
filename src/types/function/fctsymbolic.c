/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/function/fctsymbolic.h>

#include <string.h>

#include <jive/util/buffer.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/substitution.h>

static void
jive_symbolicfunction_node_fini_(jive_node * self_);

static void
jive_symbolicfunction_node_get_label_(const jive_node * self_, struct jive_buffer * buffer);

static const jive_node_attrs *
jive_symbolicfunction_node_get_attrs_(const jive_node * self);

static bool
jive_symbolicfunction_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs);

static jive_node *
jive_symbolicfunction_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_SYMBOLICFUNCTION_NODE = {
	parent : &JIVE_NODE,
	name : "SYMBOLICFUNCTION",
	fini : jive_symbolicfunction_node_fini_, /* override */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : jive_symbolicfunction_node_get_label_, /* override */
	get_attrs : jive_symbolicfunction_node_get_attrs_, /* inherit */
	match_attrs : jive_symbolicfunction_node_match_attrs_, /* override */
	check_operands : NULL,
	create : jive_symbolicfunction_node_create_, /* override */
};

static void
jive_symbolicfunction_node_init_(
	jive_symbolicfunction_node * node,
	jive_graph * graph,
	const char * fctname,
	const jive_function_type * type)
{
	node->attrs.name = jive_context_strdup(graph->context, fctname);
	jive_function_type_init(&node->attrs.type,
		type->narguments, (const jive_type **) type->argument_types,
		type->nreturns, (const jive_type **) type->return_types);

	const jive_type * rtype = type;
	jive_node_init_(node, graph->root_region,
		0, NULL, NULL,
		1, &rtype);
}

static void
jive_symbolicfunction_node_fini_(jive_node * self_)
{
	jive_symbolicfunction_node * self = (jive_symbolicfunction_node *) self_;
	
	jive_context_free(self_->graph->context, (char *)self->attrs.name);
	
	jive_function_type_fini(&self->attrs.type);
	
	jive_node_fini_(self);
}

static void
jive_symbolicfunction_node_get_label_(const jive_node * self_, struct jive_buffer * buffer)
{
	const jive_symbolicfunction_node * self = (const jive_symbolicfunction_node *) self_;
	jive_buffer_putstr(buffer, self->attrs.name);
}

static jive_node *
jive_symbolicfunction_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	const jive_symbolicfunction_node_attrs * attrs = (const jive_symbolicfunction_node_attrs *) attrs_;
	return jive_symbolicfunction_node_create(region->graph, attrs->name, &attrs->type);
}

static const jive_node_attrs *
jive_symbolicfunction_node_get_attrs_(const jive_node * self_)
{
	const jive_symbolicfunction_node * self = (const jive_symbolicfunction_node *) self_;
	
	return &self->attrs;
}

static bool
jive_symbolicfunction_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_symbolicfunction_node_attrs * first = &((const jive_symbolicfunction_node *)self)->attrs;
	const jive_symbolicfunction_node_attrs * second = (const jive_symbolicfunction_node_attrs *) attrs;

	if (!jive_type_equals(&first->type, &second->type)) return false;
	if (strcmp(first->name, second->name)) return false;

	return true;
}

jive_node *
jive_symbolicfunction_node_create(struct jive_graph * graph, const char * name, const jive_function_type * type) 
{
	jive_symbolicfunction_node * node = new jive_symbolicfunction_node;
	node->class_ = &JIVE_SYMBOLICFUNCTION_NODE;
	jive_symbolicfunction_node_init_(node, graph, name, type);
	return node;
} 

jive_output *
jive_symbolicfunction_create(struct jive_graph * graph, const char * name, const jive_function_type * type)
{
	return jive_symbolicfunction_node_create(graph, name, type)->outputs[0];
}
