/*
 * Copyright 2010 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "testnodes.h"

#include <jive/util/ptr-collection.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>

/* test node */

test_operation::~test_operation() noexcept {}

test_operation::test_operation(
	const std::vector<const jive::base::type*> & argument_types,
	const std::vector<const jive::base::type*> & result_types)
	: argument_types_(jive::detail::unique_ptr_vector_copy(argument_types))
	, result_types_(jive::detail::unique_ptr_vector_copy(result_types))
{
}

test_operation::test_operation(const test_operation & other)
	: argument_types_(jive::detail::unique_ptr_vector_copy(other.argument_types_))
	, result_types_(jive::detail::unique_ptr_vector_copy(other.result_types_))
{
}

bool
test_operation::operator==(const operation & other) const noexcept
{
	const test_operation * op =
		dynamic_cast<const test_operation *>(&other);
	return op &&
		jive::detail::ptr_container_equals(argument_types_, op->argument_types_) &&
		jive::detail::ptr_container_equals(result_types_, op->result_types_);
}

size_t
test_operation::narguments() const noexcept
{
	return argument_types_.size();
}

const jive::base::type &
test_operation::argument_type(size_t index) const noexcept
{
	return *argument_types_[index];
}

size_t
test_operation::nresults() const noexcept
{
	return result_types_.size();
}

const jive::base::type &
test_operation::result_type(size_t index) const noexcept
{
	return *result_types_[index];
}
std::string
test_operation::debug_string() const
{
	return "TEST_NODE";
}

std::unique_ptr<jive::operation>
test_operation::copy() const
{
	return std::unique_ptr<jive::operation>(new test_operation(*this));
}

jive_node *
jive_test_node_create(
	jive_region * region,
	const std::vector<const jive::base::type*> & operand_types,
	const std::vector<jive::output*> & operands,
	const std::vector<const jive::base::type*> & result_types)
{
	test_operation op(operand_types, result_types);
	return op.create_node(region, operands.size(), &operands[0]);
}

std::vector<jive::output*>
jive_test_node_create_normalized(
	jive_graph * graph,
	const std::vector<const jive::base::type*> & operand_types,
	const std::vector<jive::output*> & operands,
	const std::vector<const jive::base::type*> & result_types)
{
	jive_region * region = graph->root_region;
	if (!operands.empty())
		region = operands[0]->node()->region();

	test_operation op(operand_types, result_types);
	return jive_node_create_normalized(region, op, operands);
}
