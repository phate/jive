/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
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

class split_operation final : public base::unary_op {
public:
	virtual
	~split_operation() noexcept;

	constexpr inline
	split_operation(
		const jive_resource_class * in_class,
		const jive_resource_class * out_class) noexcept
		: in_class_(in_class)
		, out_class_(out_class)
	{
	}


	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override;

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
	jive::output * in_origin,
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
