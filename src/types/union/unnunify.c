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

unify_op::~unify_op() noexcept
{
}

bool
unify_op::operator==(const operation & other) const noexcept
{
	const unify_op * op =
		dynamic_cast<const unify_op *>(&other);
	return op && type_ == op->type_ && option_ == op->option_;
}

jive_node *
unify_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	jive_unify_node * node = new jive_unify_node(*this);

	const jive::base::type * argtypes[1] = { &argument_type(0) };
	const jive::base::type * restypes[1] = { &result_type(0) };

	jive_node_init_(node, region,
		1, argtypes, arguments,
		1, restypes);

	return node;
}

std::string
unify_op::debug_string() const
{
	char tmp[32];
	snprintf(tmp, sizeof(tmp), "UNIFY(%zd)", option());
	return tmp;
}

const jive::base::type &
unify_op::argument_type(size_t index) const noexcept
{
	return *type_.declaration()->elements[option()];
}

const jive::base::type &
unify_op::result_type(size_t index) const noexcept
{
	return type_;
}

jive_unop_reduction_path_t
unify_op::can_reduce_operand(
	const jive::output * arg) const noexcept
{
	return jive_unop_reduction_none;
}

jive::output *
unify_op::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	return nullptr;
}

std::unique_ptr<jive::operation>
unify_op::copy() const
{
	return std::unique_ptr<jive::operation>(new unify_op(*this));
}

empty_unify_op::~empty_unify_op() noexcept
{
}

bool
empty_unify_op::operator==(const operation & other) const noexcept
{
	const empty_unify_op * op =
		dynamic_cast<const empty_unify_op *>(&other);
	return op && op->declaration() == declaration();
}

jive_node *
empty_unify_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return jive_opnode_create(
		*this,
		&JIVE_EMPTY_UNIFY_NODE,
		region,
		std::initializer_list<jive::output *>{});
}

std::string
empty_unify_op::debug_string() const
{
	return "UNIFY";
}

const jive::base::type &
empty_unify_op::result_type(size_t index) const noexcept
{
	return type_;
}

std::unique_ptr<jive::operation>
empty_unify_op::copy() const
{
	return std::unique_ptr<jive::operation>(new empty_unify_op(*this));
}

}
}

const jive_node_class JIVE_UNIFY_NODE = {
	parent : &JIVE_UNARY_OPERATION,
	name : "UNIFY",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : nullptr,
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_unify_create(const jive::unn::declaration * decl, size_t option, jive::output * const argument)
{
	const jive::unn::type  unn_type(decl);
	jive::unn::unify_op op(unn_type, option);

	return jive_unary_operation_create_normalized(&JIVE_UNIFY_NODE, argument->node()->graph,
		&op, argument);
}

/* empty unify node */

const jive_node_class JIVE_EMPTY_UNIFY_NODE = {
	parent : &JIVE_NULLARY_OPERATION,
	name : "UNIFY",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : nullptr,
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_empty_unify_create(struct jive_graph * graph, const jive::unn::declaration * decl)
{
	jive::unn::empty_unify_op op(decl);

	return jive_nullary_operation_create_normalized(&JIVE_EMPTY_UNIFY_NODE, graph, &op);
}
