/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_UNION_UNNCHOOSE_H
#define JIVE_TYPES_UNION_UNNCHOOSE_H

#include <jive/types/union/unntype.h>
#include <jive/vsdg/unary.h>

namespace jive {
namespace unn {

class choose_operation final : public base::unary_op {
public:
	inline
	choose_operation(
		const jive::unn::type & type,
		size_t element) noexcept
		: type_(type)
		, element_(element)
	{
	}

	inline
	choose_operation(const choose_operation & other) noexcept = default;

	inline
	choose_operation(choose_operation && other) noexcept = default;

	virtual
	~choose_operation() noexcept;

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
	type type_;
	size_t element_;
};

}
}

jive::output *
jive_choose_create(size_t element, jive::output * operand);

#endif
