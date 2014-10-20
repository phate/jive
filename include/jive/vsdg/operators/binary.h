/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_OPERATORS_BINARY_H
#define JIVE_VSDG_OPERATORS_BINARY_H

#include <jive/common.h>
#include <jive/vsdg/node-normal-form.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators/base.h>

typedef size_t jive_binop_reduction_path_t;

enum jive_binary_operation_flags {
	jive_binary_operation_none = 0,
	jive_binary_operation_associative = 1,
	jive_binary_operation_commutative = 2
};

static inline constexpr jive_binary_operation_flags
operator|(jive_binary_operation_flags a, jive_binary_operation_flags b)
{
	return static_cast<jive_binary_operation_flags>(
		static_cast<int>(a) | static_cast<int>(b));
}

static inline constexpr jive_binary_operation_flags
operator&(jive_binary_operation_flags a, jive_binary_operation_flags b)
{
	return static_cast<jive_binary_operation_flags>(
		static_cast<int>(a) & static_cast<int>(b));
}

namespace jive {
namespace base {

/**
	\brief Binary operator
	
	Operator taking two arguments (with well-defined reduction for more
	operands if operator is associative).
*/
class binary_op : public operation {
public:
	virtual ~binary_op() noexcept;

	virtual jive_binop_reduction_path_t
	can_reduce_operand_pair(
		const jive::output * op1,
		const jive::output * op2) const noexcept = 0;

	virtual jive::output *
	reduce_operand_pair(
		jive_binop_reduction_path_t path,
		jive::output * op1,
		jive::output * op2) const = 0;

	virtual jive_binary_operation_flags
	flags() const noexcept;

	inline bool
	is_associative() const noexcept
	{
		return flags() & jive_binary_operation_associative;
	}

	inline bool
	is_commutative() const noexcept
	{
		return flags() & jive_binary_operation_commutative;
	}
};

}
}

struct jive_region;

static const jive_binop_reduction_path_t jive_binop_reduction_none = 0;
/* both operands are constants */
static const jive_binop_reduction_path_t jive_binop_reduction_constants = 1;
/* can merge both operands into single (using some "simpler" operator) */
static const jive_binop_reduction_path_t jive_binop_reduction_merge = 2;
/* part of left operand can be folded into right */
static const jive_binop_reduction_path_t jive_binop_reduction_lfold = 3;
/* part of right operand can be folded into left */
static const jive_binop_reduction_path_t jive_binop_reduction_rfold = 4;
/* left operand is neutral element */
static const jive_binop_reduction_path_t jive_binop_reduction_lneutral = 5;
/* right operand is neutral element */
static const jive_binop_reduction_path_t jive_binop_reduction_rneutral = 6;
/* both operands have common form which can be factored over op */
static const jive_binop_reduction_path_t jive_binop_reduction_factor = 7;

/* node class */

extern const jive_node_class JIVE_BINARY_OPERATION;

/* node class inheritable methods */

jive::node_normal_form *
jive_binary_operation_get_default_normal_form_(
	const jive_node_class * cls,
	jive::node_normal_form * parent,
	jive_graph * graph);

#endif
