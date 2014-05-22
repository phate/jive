/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_BITUQUOTIENT_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_BITUQUOTIENT_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitbinary_operation_class JIVE_BITUQUOTIENT_NODE_;
#define JIVE_BITUQUOTIENT_NODE (JIVE_BITUQUOTIENT_NODE_.base.base)

namespace jive {
namespace bitstring {

class uquotient_operation final : public bits_binary_operation {
public:
	virtual ~uquotient_operation() noexcept;

	inline uquotient_operation(
		const jive_bitstring_type & type) noexcept
		: bits_binary_operation(type)
	{
	}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive_output * const arguments[]) const override;

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

jive_output *
jive_bituquotient(jive_output * dividend, jive_output * divisor);

#endif
