/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_BITSHIPRODUCT_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_BITSHIPRODUCT_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_node_class JIVE_BITSHIPRODUCT_NODE;

namespace jive {
namespace bitstring {

class shiproduct_operation final : public bits_binary_operation {
public:
	virtual ~shiproduct_operation() noexcept;

	inline shiproduct_operation(const jive::bits::type & type) noexcept
		: bits_binary_operation(type)
	{}

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
};

}
}

jive::output *
jive_bitshiproduct(jive::output * factor1, jive::output * factor2);

#endif
