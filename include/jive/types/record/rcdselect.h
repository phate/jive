/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_RECORD_RCDSELECT_H
#define JIVE_TYPES_RECORD_RCDSELECT_H

#include <jive/rvsdg/unary.h>

namespace jive {
namespace rcd {

class select_operation final : public jive::unary_op {
public:
	virtual
	~select_operation() noexcept;

	inline
	select_operation(const jive::rcd::type & type, size_t element) noexcept
	: element_(element)
	, result_(type.declaration()->element(element))
	, argument_(type)
	{}

	inline
	select_operation(const select_operation & other) = default;

	inline
	select_operation(select_operation && other) = default;

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual std::string
	debug_string() const override;

	virtual const jive::port &
	argument(size_t index) const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

	virtual jive_unop_reduction_path_t
	can_reduce_operand(
		const jive::output * arg) const noexcept override;

	virtual jive::output *
	reduce_operand(
		jive_unop_reduction_path_t path,
		jive::output * arg) const override;

	inline size_t
	element() const noexcept { return element_; }

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	size_t element_;
	jive::port result_;
	jive::port argument_;
};

}
}

jive::output *
jive_select_create(size_t element, jive::output * argument);

#endif
