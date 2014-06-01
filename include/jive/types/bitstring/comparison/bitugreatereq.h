/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_COMPARISON_BITUGREATEREQ_H
#define JIVE_TYPES_BITSTRING_COMPARISON_BITUGREATEREQ_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitcomparison_operation_class JIVE_BITUGREATEREQ_NODE_;
#define JIVE_BITUGREATEREQ_NODE (JIVE_BITUGREATEREQ_NODE_.base.base)

namespace jive {
namespace bitstring {

class ugreatereq_operation final : public jive::bits_compare_operation {
public:
	virtual ~ugreatereq_operation() noexcept;

	inline ugreatereq_operation(
		const jive::bits::type & type) noexcept
		: bits_compare_operation(type)
	{
	}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override;

	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual compare_result
	reduce_constants(
		const bitstring::value_repr & arg1,
		const bitstring::value_repr & arg2) const override;

	virtual std::string
	debug_string() const override;
};

}
}

jive::output *
jive_bitugreatereq(jive::output * operand1, jive::output * operand2);

#endif
