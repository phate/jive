/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_SLICE_H
#define JIVE_TYPES_BITSTRING_SLICE_H

#include <jive/types/bitstring/type.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/unary.h>

namespace jive {
namespace bits {

class slice_op : public base::unary_op {
public:
	inline
	slice_op(
		const jive::bits::type & argument_type,
		size_t low,
		size_t high) noexcept
	: unary_op()
	, low_(low)
	, result_(jive::bits::type(high-low))
	, argument_(argument_type)
	{}

	virtual
	~slice_op() noexcept;

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual std::string
	debug_string() const override;

	/* type signature methods */
	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual const jive::port &
	argument(size_t index) const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

	/* reduction methods */
	virtual jive_unop_reduction_path_t
	can_reduce_operand(
		const jive::output * arg) const noexcept override;

	virtual jive::output *
	reduce_operand(
		jive_unop_reduction_path_t path,
		jive::output * arg) const override;

	inline size_t
	low() const noexcept
	{
		return low_;
	}

	inline size_t
	high() const noexcept
	{
		return low_ + static_cast<const jive::bits::type*>(&result_.type())->nbits();
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

	inline const type &
	argument_type() const noexcept
	{
		return *static_cast<const jive::bits::type*>(&argument_.type());
	}

private:
	size_t low_;
	jive::port result_;
	jive::port argument_;
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
