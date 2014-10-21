/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_BITASHR_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_BITASHR_H

#include <jive/types/bitstring/bitoperation-classes.h>

namespace jive {
namespace bits {

class ashr_op final : public binary_op {
public:
	virtual ~ashr_op() noexcept;

	inline ashr_op(const jive::bits::type & type) noexcept : binary_op(type) {}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override;

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
jive_bitashr(jive::output * operand, jive::output * shift);

#endif
