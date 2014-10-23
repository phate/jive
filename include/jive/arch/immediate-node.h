/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_IMMEDIATE_NODE_H
#define JIVE_ARCH_IMMEDIATE_NODE_H

#include <string.h>

#include <jive/arch/immediate-value.h>
#include <jive/arch/linker-symbol.h>
#include <jive/context.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators/nullary.h>

namespace jive {
class immediate_op final : public base::nullary_op {
public:
	virtual ~immediate_op() noexcept;

	inline constexpr
	immediate_op(jive_immediate value) noexcept
		: value_(value)
	{
	}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override;

	virtual std::string
	debug_string() const override;

	inline const jive_immediate & value() const noexcept { return value_; }

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	jive_immediate value_;
};
}

jive::output *
jive_immediate_create(
	struct jive_graph * graph,
	const jive_immediate * immediate_value);

#endif
