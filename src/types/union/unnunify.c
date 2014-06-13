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

namespace jive {
namespace unn {

unify_operation::~unify_operation() noexcept
{
}

bool
unify_operation::operator==(const operation & other) const noexcept
{
	const unify_operation * op =
		dynamic_cast<const unify_operation *>(&other);
	return op && type_ == op->type_ && option_ == op->option_;
}

jive_node *
unify_operation::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	jive_unify_node * node = new jive_unify_node(*this);
	node->class_ = &JIVE_UNIFY_NODE;

	const jive::base::type * argtypes[1] = { &argument_type(0) };
	const jive::base::type * restypes[1] = { &result_type(0) };

	jive_node_init_(node, region,
		1, argtypes, arguments,
		1, restypes);

	return node;
}

std::string
unify_operation::debug_string() const
{
	char tmp[32];
	snprintf(tmp, sizeof(tmp), "UNIFY(%zd)", option());
	return tmp;
}

const jive::base::type &
unify_operation::argument_type(size_t index) const noexcept
{
	return *type_.declaration()->elements[option()];
}

const jive::base::type &
unify_operation::result_type(size_t index) const noexcept
{
	return type_;
}

jive_unop_reduction_path_t
unify_operation::can_reduce_operand(
	const jive::output * arg) const noexcept
{
	return jive_unop_reduction_none;
}

jive::output *
unify_operation::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	return nullptr;
}

}
}

const jive_node_class JIVE_UNIFY_NODE = {
	parent : &JIVE_UNARY_OPERATION,
	name : "UNIFY",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_unary_operation_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_unify_create(const jive::unn::declaration * decl, size_t option, jive::output * const argument)
{
	const jive::unn::type  unn_type(decl);
	jive::unn::unify_operation op(unn_type, option);

	return jive_unary_operation_create_normalized(&JIVE_UNIFY_NODE, argument->node()->graph,
		&op, argument);
}

/* empty unify node */

static jive_node *
jive_empty_unify_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive::output * const operands[]);

static bool
jive_empty_unify_node_match_attrs_(const jive_node * self, const jive_node_attrs * second);

const jive_node_class JIVE_EMPTY_UNIFY_NODE = {
	parent : &JIVE_NULLARY_OPERATION,
	name : "UNIFY",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_nullary_operation_get_default_normal_form_, /* inherit */
	get_label : jive_node_get_label_, /* inherit */
	match_attrs : jive_empty_unify_node_match_attrs_, /* override */
	check_operands : jive_node_check_operands_, /* inherit */
	create : jive_empty_unify_node_create_, /* override */
};

static void
jive_empty_unify_node_init_(jive_empty_unify_node * self,
	struct jive_region * region, const jive::unn::declaration * decl)
{
	jive::unn::type type(decl);
	const jive::base::type * type_ptr = &type;
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
	size_t noperands, jive::output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 0);
	const jive::unn::empty_unify_operation * attrs = (const jive::unn::empty_unify_operation *) attrs_;
	
	jive_empty_unify_node * node = new jive_empty_unify_node(*attrs);
	node->class_ = &JIVE_EMPTY_UNIFY_NODE;
	jive_empty_unify_node_init_(node, region, attrs->declaration());
	
	return node;
}

jive::output *
jive_empty_unify_create(struct jive_graph * graph, const jive::unn::declaration * decl)
{
	jive::unn::empty_unify_operation op(decl);

	return jive_nullary_operation_create_normalized(&JIVE_EMPTY_UNIFY_NODE, graph, &op);
}
