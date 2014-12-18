/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_BITXOR_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_BITXOR_H

#include <jive/types/bitstring/bitoperation-classes.h>
#include <jive/types/bitstring/type.h>

namespace jive {
namespace bits {

class xor_op final : public binary_op {
public:
	virtual ~xor_op() noexcept;

	inline xor_op(const jive::bits::type & type, size_t arity = 2) noexcept
		: binary_op(type, arity)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;
	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual value_repr
	reduce_constants(
		const value_repr & arg1,
		const value_repr & arg2) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

}
}

jive::output *
jive_bitxor(size_t noperands, jive::output * const * operands);

#endif
