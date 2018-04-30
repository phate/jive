/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_REGVALUE_H
#define JIVE_ARCH_REGVALUE_H

#include <jive/arch/registers.h>
#include <jive/rvsdg/nullary.h>
#include <jive/rvsdg/simple-node.h>

namespace jive {

class register_class;

class regvalue_op final : public nullary_op {
public:
	virtual
	~regvalue_op() noexcept;

	inline
	regvalue_op(
		const nullary_op & op,
		const register_class * regcls) noexcept
	: nullary_op(regcls)
	, operation_(std::move(op.copy()))
	{}

	inline
	regvalue_op(const regvalue_op & other)
	: nullary_op(other)
	, operation_(std::move(other.operation_->copy()))
	{}

	inline
	regvalue_op(regvalue_op && other)
	: nullary_op(other)
	, operation_(std::move(other.operation_))
	{}

	regvalue_op &
	operator=(const regvalue_op & other)
	{
		if (&other == this)
			return *this;

		operation_ = std::move(other.operation_->copy());
		return *this;
	}

	regvalue_op &
	operator=(regvalue_op && other)
	{
		if (&other == this)
			return *this;

		operation_ = std::move(other.operation_);
		return *this;
	}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual std::string
	debug_string() const override;

	inline const register_class *
	regcls() const
	{
		return static_cast<const register_class*>(result(0).rescls());
	}

	inline const nullary_op &
	operation() const noexcept
	{
		return *static_cast<const nullary_op*>(operation_.get());
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

	static inline jive::output *
	create(
		jive::region * region,
		const nullary_op & operation,
		const register_class * regcls)
	{
		regvalue_op op(operation, regcls);
		return simple_node::create_normalized(region, op, {})[0];
	}

private:
	std::unique_ptr<jive::operation> operation_;
};

static inline bool
is_regvalue_op(const jive::operation & op) noexcept
{
	return dynamic_cast<const regvalue_op*>(&op) != nullptr;
}

static inline bool
is_regvalue_node(const jive::node * node) noexcept
{
	return is<regvalue_op>(node);
}

}

#endif
