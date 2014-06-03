/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_CONCAT_H
#define JIVE_TYPES_BITSTRING_CONCAT_H

#include <jive/types/bitstring/type.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators.h>

extern const jive_binary_operation_class JIVE_BITCONCAT_NODE_;
#define JIVE_BITCONCAT_NODE (JIVE_BITCONCAT_NODE_.base)

namespace jive {
namespace bitstring {

class concat_operation final : public jive::binary_operation {
public:
	virtual ~concat_operation() noexcept;

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override;

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
};

}
}

jive::output *
jive_bitconcat(size_t noperands, jive::output * const * operands);

#endif
