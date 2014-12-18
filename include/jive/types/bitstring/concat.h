/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_CONCAT_H
#define JIVE_TYPES_BITSTRING_CONCAT_H

#include <vector>

#include <jive/types/bitstring/type.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators.h>

namespace jive {
namespace bits {

class concat_op final : public jive::base::binary_op {
public:
	virtual ~concat_op() noexcept;

	explicit inline
	concat_op(std::vector<type> argument_types)
		: argument_types_(std::move(argument_types))
		, result_type_(aggregate_arguments(argument_types_))
	{
	}

	inline
	concat_op(const concat_op & other)
		: argument_types_(other.argument_types_)
		, result_type_(other.result_type_)
	{
	}

	inline
	concat_op(concat_op && other) noexcept = default;

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
	/* reduction methods */
	virtual jive_binop_reduction_path_t
	can_reduce_operand_pair(
		const jive::output * arg1,
		const jive::output * arg2) const noexcept override;

	virtual jive::output *
	reduce_operand_pair(
		jive_binop_reduction_path_t path,
		jive::output * arg1,
		jive::output * arg2) const override;

	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual std::string
	debug_string() const override;

	inline const std::vector<type> &
	argument_types() const noexcept
	{
		return argument_types_;
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	static type
	aggregate_arguments(
		const std::vector<type>& argument_types) noexcept;

	std::vector<type> argument_types_;
	type result_type_;
};

}
}

jive::output *
jive_bitconcat(size_t noperands, jive::output * const * operands);

#endif
