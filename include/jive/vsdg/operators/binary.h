/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
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
		const jive::oport * op1,
		const jive::oport * op2) const noexcept = 0;

	virtual jive::oport *
	reduce_operand_pair(
		jive_binop_reduction_path_t path,
		jive::oport * op1,
		jive::oport * op2) const = 0;

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

class flattened_binary_op final : public operation {
public:
	virtual ~flattened_binary_op() noexcept;

	inline
	flattened_binary_op(
		std::unique_ptr<binary_op> op,
		size_t narguments) noexcept
		: op_(std::move(op))
		, narguments_(narguments)
	{
	}

	inline
	flattened_binary_op(
		const binary_op & op,
		size_t narguments)
		: op_(std::unique_ptr<binary_op>(static_cast<binary_op *>(op.copy().release())))
		, narguments_(narguments)
	{
	}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual size_t
	narguments() const noexcept override;

	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;
	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;

	inline const binary_op &
	bin_operation() const noexcept
	{
		return *op_;
	}

private:
	std::unique_ptr<binary_op> op_;
	size_t narguments_;
};

}
}

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

#endif
