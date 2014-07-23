/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_SUBROUTINE_NODES_H
#define JIVE_ARCH_SUBROUTINE_NODES_H

#include <jive/vsdg/anchor.h>
#include <jive/vsdg/node.h>

struct jive_subroutine_deprecated;

namespace jive {

class subroutine_head_op final : public region_head_op {
public:
	virtual
	~subroutine_head_op() noexcept;

	virtual size_t
	nresults() const noexcept override;

	virtual const base::type &
	result_type(size_t index) const noexcept override;

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override;

	virtual std::string
	debug_string() const override;
};

class subroutine_tail_op final : public region_tail_op {
public:
	virtual
	~subroutine_tail_op() noexcept;

	virtual size_t
	narguments() const noexcept override;

	virtual const base::type &
	argument_type(size_t index) const noexcept override;

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override;

	virtual std::string
	debug_string() const override;
};

class subroutine_op final : public region_anchor_op {
public:
	virtual
	~subroutine_op() noexcept;

	inline constexpr subroutine_op(
		jive_subroutine_deprecated * subroutine) noexcept
		: subroutine_(subroutine)
	{
	}

	subroutine_op(const subroutine_op & other);

	subroutine_op(subroutine_op && other) noexcept = default;

	virtual size_t
	nresults() const noexcept override;

	virtual const base::type &
	result_type(size_t index) const noexcept override;

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override;

	virtual std::string
	debug_string() const override;

	inline jive_subroutine_deprecated *
	subroutine() const noexcept
	{
		return subroutine_;
	}

private:
	jive_subroutine_deprecated * subroutine_;
};

}

typedef jive::operation_node<jive::subroutine_op> jive_subroutine_node;
typedef jive::operation_node<jive::subroutine_head_op> jive_subroutine_enter_node;
typedef jive::operation_node<jive::subroutine_tail_op> jive_subroutine_leave_node;

extern const jive_node_class JIVE_SUBROUTINE_ENTER_NODE;
extern const jive_node_class JIVE_SUBROUTINE_LEAVE_NODE;
extern const jive_node_class JIVE_SUBROUTINE_NODE;

jive_node *
jive_subroutine_node_create(
	jive_region * subroutine_region,
	jive_subroutine_deprecated * subroutine);

#endif
