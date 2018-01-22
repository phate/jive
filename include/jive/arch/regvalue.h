/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_REGVALUE_H
#define JIVE_ARCH_REGVALUE_H

#include <jive/arch/registers.h>
#include <jive/rvsdg/simple-node.h>

namespace jive {

class register_class;

class regvalue_op final : public simple_op {
public:
	virtual
	~regvalue_op() noexcept;

	inline
	regvalue_op(const register_class * regcls) noexcept
	: simple_op({regcls}, {regcls})
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual std::string
	debug_string() const override;

	inline const register_class *
	regcls() const
	{
		return static_cast<const register_class*>(argument(0).rescls());
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

	static inline jive::output *
	create(jive::output * value, const register_class * regcls)
	{
		regvalue_op op(regcls);
		return simple_node::create_normalized(value->region(), op, {value})[0];
	}
};

static inline bool
is_regvalue_op(const jive::operation & op) noexcept
{
	return dynamic_cast<const regvalue_op*>(&op) != nullptr;
}

static inline bool
is_regvalue_node(const jive::node * node) noexcept
{
	return is_opnode<regvalue_op>(node);
}

}

#endif
