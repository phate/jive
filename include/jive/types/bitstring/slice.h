/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_SLICE_H
#define JIVE_TYPES_BITSTRING_SLICE_H

#include <jive/types/bitstring/type.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators.h>

namespace jive {
namespace bits {

class slice_op : public base::unary_op {
public:
	inline constexpr
	slice_op(
		const jive::bits::type & argument_type,
		size_t low, size_t high) noexcept
		: argument_type_(argument_type)
		, result_type_(high - low)
		, low_(low)
	{
	}

	virtual
	~slice_op() noexcept;

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override;

	virtual std::string
	debug_string() const override;

	/* type signature methods */
	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	/* reduction methods */
	virtual jive_unop_reduction_path_t
	can_reduce_operand(
		const jive::output * arg) const noexcept override;

	virtual jive::output *
	reduce_operand(
		jive_unop_reduction_path_t path,
		jive::output * arg) const override;

	inline size_t
	low() const noexcept { return low_; }

	inline size_t
	high() const noexcept { return low_ + result_type_.nbits(); }

	virtual std::unique_ptr<jive::operation>
	copy() const override;

	inline const type &
	argument_type() const noexcept
	{
		return argument_type_;
	}

private:
	jive::bits::type argument_type_;
	jive::bits::type result_type_;
	size_t low_;
};

}
}

/**
	\brief Create bitslice
	\param operand Input value
	\param low Low bit
	\param high High bit
	\returns Bitstring value representing slice
	
	Convenience function that either creates a new slice or
	returns the output handle of an existing slice.
*/
jive::output *
jive_bitslice(jive::output * operand, size_t low, size_t high);

#endif
