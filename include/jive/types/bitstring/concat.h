/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_CONCAT_H
#define JIVE_TYPES_BITSTRING_CONCAT_H

#include <vector>

#include <jive/rvsdg/node.h>
#include <jive/rvsdg/binary.h>
#include <jive/types/bitstring/type.h>

namespace jive {

class bitconcat_op final : public jive::binary_op {
public:
	virtual
	~bitconcat_op() noexcept;

	explicit inline
	bitconcat_op(const std::vector<bittype> & types)
	: binary_op(to_ports(types), {aggregate_arguments(types)})
	{}

	virtual jive_binop_reduction_path_t
	can_reduce_operand_pair(
		const jive::output * arg1,
		const jive::output * arg2) const noexcept override;

	virtual jive::output *
	reduce_operand_pair(
		jive_binop_reduction_path_t path,
		jive::output * arg1,
		jive::output * arg2) const override;

	virtual enum jive::binary_op::flags
	flags() const noexcept override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	static bittype
	aggregate_arguments(const std::vector<bittype> & types) noexcept;

	static std::vector<jive::port>
	to_ports(const std::vector<bittype> & types);
};

}

jive::output *
jive_bitconcat(const std::vector<jive::output*> & operands);

#endif
