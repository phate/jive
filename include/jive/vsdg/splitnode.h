/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_SPLITNODE_H
#define JIVE_VSDG_SPLITNODE_H

/* auxiliary node to represent a "split" of the same value */

#include <jive/common.h>

#include <jive/vsdg/node.h>
#include <jive/vsdg/operators/unary.h>

struct jive_resource_class;

namespace jive {

class split_operation final : public unary_operation {
public:
	constexpr split_operation(
		const jive_resource_class * in_class,
		const jive_resource_class * out_class) noexcept
		: in_class_(in_class)
		, out_class_(out_class)
	{
	}

	inline const jive_resource_class * in_class() const noexcept { return in_class_; }
	inline const jive_resource_class * out_class() const noexcept { return out_class_; }

private:
	const jive_resource_class * in_class_;
	const jive_resource_class * out_class_;
};

}

typedef jive::operation_node<jive::split_operation> jive_splitnode;

extern const jive_node_class JIVE_SPLITNODE;

jive_node *
jive_splitnode_create(struct jive_region * region,
	const jive::base::type * in_type,
	struct jive_output * in_origin,
	const struct jive_resource_class * in_class,
	const jive::base::type * out_type,
	const struct jive_resource_class * out_class);

JIVE_EXPORTED_INLINE jive_splitnode *
jive_splitnode_cast(jive_node * self)
{
	if (self->class_ == &JIVE_SPLITNODE)
		return (jive_splitnode *) self;
	else
		return 0;
}

#endif
