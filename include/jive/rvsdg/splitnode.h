/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_RVSDG_SPLITNODE_H
#define JIVE_RVSDG_SPLITNODE_H

/* auxiliary node to represent a "split" of the same value */

#include <jive/common.h>

#include <jive/rvsdg/node.h>
#include <jive/rvsdg/unary.h>

namespace jive {

class resource_class;

class split_operation final : public jive::unary_op {
public:
	virtual
	~split_operation() noexcept;

	inline
	split_operation(
		const resource_class * in_class,
		const resource_class * out_class) noexcept
	: result_(out_class)
	, argument_(in_class)
	{}

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

	inline const resource_class *
	in_class() const noexcept
	{
		return argument_.rescls();
	}

	inline const resource_class *
	out_class() const noexcept
	{
		return result_.rescls();
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	jive::port result_;
	jive::port argument_;
};

}

jive::node *
jive_splitnode_create(
	jive::region * region,
	jive::output * in_origin,
	const jive::resource_class * in_class,
	const jive::resource_class * out_class);

#endif
