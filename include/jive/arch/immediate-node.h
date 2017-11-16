/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_IMMEDIATE_NODE_H
#define JIVE_ARCH_IMMEDIATE_NODE_H

#include <string.h>

#include <jive/arch/immediate-value.h>
#include <jive/arch/linker-symbol.h>
#include <jive/rvsdg/node.h>
#include <jive/rvsdg/nullary.h>

namespace jive {
class immediate_op final : public base::nullary_op {
public:
	virtual ~immediate_op() noexcept;

	inline constexpr
	immediate_op(jive::immediate value) noexcept
		: value_(value)
	{
	}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

	virtual std::string
	debug_string() const override;

	inline const jive::immediate & value() const noexcept { return value_; }

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	jive::immediate value_;
};
}

jive::output *
jive_immediate_create(
	jive::region * region,
	const jive::immediate * immediate_value);

#endif
