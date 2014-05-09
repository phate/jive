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

namespace jive {
class immediate_operation final : public operation {
public:
	virtual ~immediate_operation() noexcept;

	inline constexpr
	immediate_operation(jive_immediate value) noexcept
		: value_(value)
	{
	}

	inline const jive_immediate & value() const noexcept { return value_; }

private:
	jive_immediate value_;
};
}

typedef jive::operation_node<jive::immediate_operation> jive_immediate_node;

extern const jive_node_class JIVE_IMMEDIATE_NODE;

jive_output *
jive_immediate_create(
	struct jive_graph * graph,
	const jive_immediate * immediate_value);

JIVE_EXPORTED_INLINE jive_immediate_node *
jive_immediate_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_IMMEDIATE_NODE))
		return (jive_immediate_node *) node;
	else
		return 0;
}

#endif
