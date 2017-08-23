/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/union/unnunify.h>

#include <jive/types/union/unntype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/simple_node.h>

namespace jive {
namespace unn {

unify_op::~unify_op() noexcept
{
}

bool
unify_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const unify_op*>(&other);
	return op
	    && option_ == op->option_
	    && result_ == op->result_
	    && argument_ == op->argument_;
}

std::string
unify_op::debug_string() const
{
	return detail::strfmt("UNIFY(", option(), ")");
}

const jive::port &
unify_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return argument_;
}

const jive::port &
unify_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return result_;
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
	auto op = dynamic_cast<const empty_unify_op *>(&other);
	return op && op->port_ == port_;
}
std::string
empty_unify_op::debug_string() const
{
	return "UNIFY";
}

const jive::port &
empty_unify_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return port_;
}

std::unique_ptr<jive::operation>
empty_unify_op::copy() const
{
	return std::unique_ptr<jive::operation>(new empty_unify_op(*this));
}

}
}

jive::output *
jive_unify_create(const jive::unn::declaration * decl, size_t option, jive::output * const argument)
{
	const jive::unn::type  unn_type(decl);
	jive::unn::unify_op op(unn_type, option);
	return jive::create_normalized(argument->region(), op, {argument})[0];
}

/* empty unify node */

jive::output *
jive_empty_unify_create(struct jive::region * region, const jive::unn::declaration * decl)
{
	jive::unn::empty_unify_op op(decl);
	return jive::create_normalized(region, op, {})[0];
}
