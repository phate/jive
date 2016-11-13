/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_RECORD_RCDSELECT_H
#define JIVE_TYPES_RECORD_RCDSELECT_H

#include <jive/vsdg/operators/unary.h>

namespace jive {
namespace rcd {

class select_operation final : public base::unary_op {
public:
	inline
	select_operation(
		const jive::rcd::type & type,
		size_t element) noexcept
		: type_(type)
		, element_(element)
	{
	}

	inline
	select_operation(const select_operation & other) noexcept = default;

	inline
	select_operation(select_operation && other) noexcept = default;

	virtual
	~select_operation() noexcept;

	virtual bool
	operator==(const operation & other) const noexcept override;
	virtual std::string
	debug_string() const override;

	/* type signature methods */
	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	/* reduction methods */
	virtual jive_unop_reduction_path_t
	can_reduce_operand(
		const jive::oport * arg) const noexcept override;

	virtual jive::oport *
	reduce_operand(
		jive_unop_reduction_path_t path,
		jive::oport * arg) const override;

	inline size_t
	element() const noexcept { return element_; }

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	type type_;
	size_t element_;
};

}
}

jive::output *
jive_select_create(size_t element, jive::output * argument);

#endif
