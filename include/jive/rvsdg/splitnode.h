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

class split_op final : public jive::unary_op {
public:
	virtual
	~split_op() noexcept;

	inline
	split_op(
		const resource_class * srcrescls,
		const resource_class * dstrescls) noexcept
	: result_(dstrescls)
	, argument_(srcrescls)
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
	srcrescls() const noexcept
	{
		return argument_.rescls();
	}

	inline const resource_class *
	dstrescls() const noexcept
	{
		return result_.rescls();
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

	static inline jive::unary_normal_form *
	normal_form(jive::graph * graph)
	{
		return static_cast<jive::unary_normal_form*>(graph->node_normal_form(typeid(split_op)));
	}

	static inline jive::output *
	create(
		jive::output * operand,
		const resource_class * srcrescls,
		const resource_class * dstrescls)
	{
		auto nf = normal_form(operand->region()->graph());
		if (nf->get_mutable())
			operand->region()->graph()->mark_denormalized();

		split_op op(srcrescls, dstrescls);
		return create_normalized(operand->region(), op, {operand})[0];
	}

private:
	jive::port result_;
	jive::port argument_;
};

static inline bool
is_split_op(const jive::operation & op) noexcept
{
	return dynamic_cast<const jive::split_op*>(&op) != nullptr;
}

static inline bool
is_split_node(const jive::node * node) noexcept
{
	return is_opnode<split_op>(node);
}

}

#endif
