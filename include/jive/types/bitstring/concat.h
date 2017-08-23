/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_CONCAT_H
#define JIVE_TYPES_BITSTRING_CONCAT_H

#include <vector>

#include <jive/types/bitstring/type.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/binary.h>

namespace jive {
namespace bits {

class concat_op final : public jive::base::binary_op {
public:
	virtual ~concat_op() noexcept;

	explicit inline
	concat_op(std::vector<type> types)
	: binary_op()
	, result_(aggregate_arguments(types))
	{
		for (const auto & type : types)
			arguments_.push_back({type});
	}

	inline
	concat_op(const concat_op & other) = default;

	inline
	concat_op(concat_op && other) = default;

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual size_t
	narguments() const noexcept override;

	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual const jive::port &
	argument(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

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

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	static type
	aggregate_arguments(
		const std::vector<type>& argument_types) noexcept;

	jive::port result_;
	std::vector<jive::port> arguments_;
};

}
}

jive::output *
jive_bitconcat(const std::vector<jive::output*> & operands);

#endif
