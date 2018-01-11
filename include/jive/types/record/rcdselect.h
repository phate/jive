/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_RECORD_RCDSELECT_H
#define JIVE_TYPES_RECORD_RCDSELECT_H

#include <jive/rvsdg/simple-node.h>
#include <jive/rvsdg/unary.h>

namespace jive {
namespace rcd {

class select_op final : public jive::unary_op {
public:
	virtual
	~select_op() noexcept;

private:
	inline
	select_op(const jive::rcd::type & type, size_t index) noexcept
	: index_(index)
	, result_(type.declaration()->element(index))
	, argument_(type)
	{}

public:
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

	inline size_t
	index() const noexcept
	{
		return index_;
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

	static inline jive::output *
	create(jive::output * operand, size_t index)
	{
		auto rt = dynamic_cast<const jive::rcd::type*>(&operand->type());
		if (!rt) throw type_error("rcd", operand->type().debug_string());

		select_op op(*rt, index);
		return jive::create_normalized(operand->region(), op, {operand})[0];
	}

private:
	size_t index_;
	jive::port result_;
	jive::port argument_;
};

static inline bool
is_select_op(const jive::operation & op) noexcept
{
	return dynamic_cast<const select_op*>(&op) != nullptr;
}

static inline bool
is_select_node(const jive::node * node) noexcept
{
	return is_opnode<select_op>(node);
}

}}

#endif
