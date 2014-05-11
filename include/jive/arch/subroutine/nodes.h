/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_SUBROUTINE_NODES_H
#define JIVE_ARCH_SUBROUTINE_NODES_H

#include <jive/vsdg/node.h>

struct jive_subroutine_deprecated;

namespace jive {

class subroutine_operation final : public operation {
public:
	inline constexpr subroutine_operation(
		jive_subroutine_deprecated * subroutine) noexcept
		: subroutine_(subroutine)
	{
	}

	inline jive_subroutine_deprecated * subroutine() const noexcept
	{
		return subroutine_;
	}
private:
	jive_subroutine_deprecated * subroutine_;
};

class subroutine_enter_operation final : public operation {
};

class subroutine_leave_operation final : public operation {
};

}

typedef jive::operation_node<jive::subroutine_operation> jive_subroutine_node;
typedef jive::operation_node<jive::subroutine_enter_operation> jive_subroutine_enter_node;
typedef jive::operation_node<jive::subroutine_leave_operation> jive_subroutine_leave_node;

extern const jive_node_class JIVE_SUBROUTINE_ENTER_NODE;
extern const jive_node_class JIVE_SUBROUTINE_LEAVE_NODE;
extern const jive_node_class JIVE_SUBROUTINE_NODE;

JIVE_EXPORTED_INLINE jive_subroutine_node *
jive_subroutine_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_SUBROUTINE_NODE))
		return (jive_subroutine_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_subroutine_enter_node *
jive_subroutine_enter_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_SUBROUTINE_ENTER_NODE))
		return (jive_subroutine_enter_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_subroutine_leave_node *
jive_subroutine_leave_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_SUBROUTINE_LEAVE_NODE))
		return (jive_subroutine_leave_node *) node;
	else
		return NULL;
}

jive_node *
jive_subroutine_enter_node_create(jive_region * region);

jive_node *
jive_subroutine_leave_node_create(jive_region * region, jive_output * control_transfer);

jive_node *
jive_subroutine_node_create(
	jive_region * subroutine_region,
	jive_subroutine_deprecated * subroutine);

#endif
