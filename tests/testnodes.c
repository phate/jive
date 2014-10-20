/*
 * Copyright 2010 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "testnodes.h"

#include <jive/util/ptr-collection.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>

/* test node */

test_operation::~test_operation() noexcept {}

test_operation::test_operation(
	size_t narguments, const jive::base::type * const argument_types[],
	size_t nresults, const jive::base::type * const result_types[])
	: argument_types_(jive::detail::unique_ptr_vector_copy(
		jive::detail::make_array_slice(argument_types, narguments)))
	, result_types_(jive::detail::unique_ptr_vector_copy(
		jive::detail::make_array_slice(result_types, nresults)))
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

jive_node *
test_operation::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return jive_opnode_create(
		*this,
		&JIVE_TEST_NODE,
		region,
		arguments, arguments + narguments);
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

const jive_node_class JIVE_TEST_NODE = {
	parent : &JIVE_NODE,
	name : "TEST_NODE",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : nullptr,
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive_node *
jive_test_node_create(struct jive_region * region,
	size_t noperands, const jive::base::type * const operand_types[], jive::output * const operands[],
	size_t nresults, const jive::base::type * const result_types[])
{
	test_operation op(noperands, operand_types, nresults, result_types);
	return op.create_node(region, noperands, operands);
}

void
jive_test_node_create_normalized(jive_graph * graph, size_t noperands,
	const jive::base::type * const operand_types[], jive::output * const operands[], size_t nresults,
	const jive::base::type * const result_types[], jive::output * results[])
{
	test_operation op(noperands, operand_types, nresults, result_types);

	jive_node_create_normalized(
		graph, op, std::vector<jive::output *>(operands, operands + noperands));
}
