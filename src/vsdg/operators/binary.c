/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/operators/binary.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/operators/binary-normal-form.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace base {

binary_op::~binary_op() noexcept {}

jive_binary_operation_flags
binary_op::flags() const noexcept
{
	return jive_binary_operation_none;
}


flattened_binary_op::~flattened_binary_op() noexcept
{
}

bool
flattened_binary_op::operator==(const operation & other) const noexcept
{
	const flattened_binary_op * other_op =
		dynamic_cast<const flattened_binary_op *>(&other);
	return other_op && *other_op->op_ == *op_ && other_op->narguments_ == narguments_;
}

size_t
flattened_binary_op::narguments() const noexcept
{
	return narguments_;
}

const jive::base::type &
flattened_binary_op::argument_type(size_t index) const noexcept
{
	return op_->argument_type(0);
}

size_t
flattened_binary_op::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
flattened_binary_op::result_type(size_t index) const noexcept
{
	return op_->result_type(0);
}
std::string
flattened_binary_op::debug_string() const
{
	return op_->debug_string();
}

std::unique_ptr<jive::operation>
flattened_binary_op::copy() const
{
	std::unique_ptr<binary_op> copied_op(
		static_cast<binary_op *>(op_->copy().release()));
	return std::unique_ptr<jive::operation>(
		new flattened_binary_op(std::move(copied_op), narguments_));
}

}
}

/* node class */

/* node class inheritable methods */

jive::node_normal_form *
jive_binary_operation_get_default_normal_form_(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive::graph * graph)
{
	jive::binary_normal_form * nf = new jive::binary_normal_form(operator_class,  parent, graph);

	return nf;
}

jive::node_normal_form *
jive_flattened_binary_operation_get_default_normal_form_(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive::graph * graph)
{
	return new jive::flattened_binary_normal_form(operator_class,  parent, graph);
}

static void  __attribute__((constructor))
register_node_normal_form(void)
{
	jive::node_normal_form::register_factory(
		typeid(jive::base::binary_op),
		jive_binary_operation_get_default_normal_form_);
	jive::node_normal_form::register_factory(
		typeid(jive::base::flattened_binary_op),
		jive_flattened_binary_operation_get_default_normal_form_);
}
