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
	bitconcat_op(std::vector<bittype> types)
	: binary_op()
	, result_(aggregate_arguments(types))
	{
		for (const auto & type : types)
			arguments_.push_back({type});
	}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual size_t
	narguments() const noexcept override;

	virtual const jive::port &
	argument(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

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
	aggregate_arguments(
		const std::vector<bittype>& argument_types) noexcept;

	jive::port result_;
	std::vector<jive::port> arguments_;
};

}

jive::output *
jive_bitconcat(const std::vector<jive::output*> & operands);

#endif
