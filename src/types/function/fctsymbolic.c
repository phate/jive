/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
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

namespace jive {
namespace fct {

symfunction_operation::~symfunction_operation() noexcept {}

symfunction_operation::symfunction_operation(
	const std::string & name,
	const jive_function_type & type)
	: name_(name)
	, type_(type)
{
}

symfunction_operation::symfunction_operation(
	const std::string && name,
	const jive_function_type && type) noexcept
	: name_(std::move(name))
	, type_(std::move(type))
{
}

}
}

static void
jive_symbolicfunction_node_fini_(jive_node * self_);

static void
jive_symbolicfunction_node_get_label_(const jive_node * self_, struct jive_buffer * buffer);

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
	const jive_type * rtype = type;
	jive_node_init_(node, graph->root_region,
		0, NULL, NULL,
		1, &rtype);
}

static void
jive_symbolicfunction_node_fini_(jive_node * self_)
{
	jive_symbolicfunction_node * self = (jive_symbolicfunction_node *) self_;
	jive_node_fini_(self);
}

static void
jive_symbolicfunction_node_get_label_(const jive_node * self_, struct jive_buffer * buffer)
{
	const jive_symbolicfunction_node * self = (const jive_symbolicfunction_node *) self_;
	jive_buffer_putstr(buffer, self->operation().name().c_str());
}

static jive_node *
jive_symbolicfunction_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	const jive::fct::symfunction_operation * attrs = (const jive::fct::symfunction_operation *) attrs_;

	jive_symbolicfunction_node * node = new jive_symbolicfunction_node(*attrs);
	node->class_ = &JIVE_SYMBOLICFUNCTION_NODE;
	jive_symbolicfunction_node_init_(node, region->graph, attrs->name().c_str(), &attrs->type());
	return node;
}

static bool
jive_symbolicfunction_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive::fct::symfunction_operation * first =
		&((const jive_symbolicfunction_node *)self)->operation();
	const jive::fct::symfunction_operation * second =
		(const jive::fct::symfunction_operation *) attrs;

	return
		first->name() == second->name() &&
		jive_type_equals(&first->type(), &second->type());
}

jive_node *
jive_symbolicfunction_node_create(
	struct jive_graph * graph, const char * name, const jive_function_type * type)
{
	jive_symbolicfunction_node * node = new jive_symbolicfunction_node(
		jive::fct::symfunction_operation(name, *type));
	node->class_ = &JIVE_SYMBOLICFUNCTION_NODE;
	jive_symbolicfunction_node_init_(node, graph, name, type);
	return node;
}

jive_output *
jive_symbolicfunction_create(
	struct jive_graph * graph, const char * name, const jive_function_type * type)
{
	return jive_symbolicfunction_node_create(graph, name, type)->outputs[0];
}
