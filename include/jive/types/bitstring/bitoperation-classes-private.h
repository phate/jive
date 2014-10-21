/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_BITOPERATION_CLASSES_PRIVATE_H
#define JIVE_TYPES_BITSTRING_BITOPERATION_CLASSES_PRIVATE_H

#include <jive/vsdg/node-private.h>

namespace jive {
namespace bits {
namespace detail {

template<typename Op>
static inline jive_node *
unop_create(
	const Op & operation,
	jive_region * region,
	jive::output * argument)
{
	jive_node * node = jive::create_operation_node(operation);

	const jive::base::type * types[1] = {&operation.type()};

	jive_node_init_(
		node, region,
		1, types, &argument,
		1, types);

	return node;
}

template<typename Op>
static inline jive_node *
binop_create(
	const Op & operation,
	jive_region * region,
	size_t narguments,
	jive::output * const * arguments)
{
	JIVE_DEBUG_ASSERT(narguments == operation.narguments());

	jive_node * node = jive::create_operation_node(operation);

	const jive::base::type * argument_types[narguments];
	for(size_t n = 0; n < narguments; n++)
		argument_types[n] = &operation.argument_type(n);

	const jive::base::type * result_types[1] = {&operation.result_type(0)};
	jive_node_init_(
		node, region,
		narguments, argument_types, arguments,
		1, result_types);

	return node;
}

template<typename Op>
static inline jive_node *
binop_create(
	const Op & operation,
	jive_region * region,
	jive::output * arg1,
	jive::output * arg2)
{
	JIVE_DEBUG_ASSERT(2 == operation.narguments());

	jive_node * node = jive::create_operation_node(operation);

	jive::output * arguments[2] = {arg1, arg2};
	const jive::base::type * argument_types[2];
	for(size_t n = 0; n < 2; n++)
		argument_types[n] = &operation.argument_type(n);

	const jive::base::type * result_types[1] = {&operation.result_type(0)};
	jive_node_init_(
		node, region,
		2, argument_types, arguments,
		1, result_types);

	return node;
}

}
}
}

/* bitcomparison operation class inhertiable members */

void
jive_bitcomparison_operation_check_operands_(const jive_node_class * cls,
	const jive_node_attrs * attrs, size_t noperands, jive::output * const operands[],
	jive_context * context);

#endif
