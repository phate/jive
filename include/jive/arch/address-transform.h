/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_ADDRESS_TRANSFORM_H
#define JIVE_ARCH_ADDRESS_TRANSFORM_H

#include <jive/arch/address.h>
#include <jive/arch/call.h>
#include <jive/arch/load.h>
#include <jive/arch/store.h>
#include <jive/common.h>
#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/unary.h>
#include <jive/types/function/fctapply.h>
#include <jive/types/function/fctlambda.h>

namespace jive {

/* addr2bit operation */

class addr2bit_op final : public jive::unary_op {
public:
	virtual
	~addr2bit_op() noexcept;

private:
	addr2bit_op(size_t nbits, const jive::type & type);

	addr2bit_op(addr2bit_op &&) noexcept = default;

	addr2bit_op(const addr2bit_op &) = default;

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
	nbits() const noexcept
	{
		return nbits_;
	}

	inline const jive::type &
	original_type() const noexcept
	{
		return argument_.type();
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

	static inline jive::output *
	create(jive::output * operand, size_t nbits, const jive::type & type)
	{
		addr2bit_op op(nbits, type);
		return jive::create_normalized(operand->region(), op, {operand})[0];
	}

private:
	size_t nbits_;
	jive::port result_;
	jive::port argument_;
};

static inline bool
is_addr2bit_op(const jive::operation & op) noexcept
{
	return dynamic_cast<const jive::addr2bit_op*>(&op) != nullptr;
}

static inline bool
is_addr2bit_node(const jive::node * node) noexcept
{
	return is_opnode<addr2bit_op>(node);
}

/* bit2addr operator */

class bit2addr_op final : public jive::unary_op {
public:
	virtual
	~bit2addr_op() noexcept;

private:
	bit2addr_op(
		size_t nbits,
		const jive::type & type);

	bit2addr_op(bit2addr_op &&) = default;

	bit2addr_op(const bit2addr_op &) = default;

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
	nbits() const noexcept
	{
		return nbits_;
	}

	inline const jive::type &
	original_type() const noexcept
	{
		return result_.type();
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

	static inline jive::output *
	create(jive::output * operand, size_t nbits, const jive::type & type)
	{
		bit2addr_op op(nbits, type);
		return jive::create_normalized(operand->region(), op, {operand})[0];
	}

private:
	size_t nbits_;
	jive::port result_;
	jive::port argument_;
};

static inline bool
is_bit2addr_op(const jive::operation & op) noexcept
{
	return dynamic_cast<const bit2addr_op*>(&op) != nullptr;
}

static inline bool
is_bit2addr_node(const jive::node * node) noexcept
{
	return is_opnode<bit2addr_op>(node);
}

class memlayout_mapper;

}

/* reductions */

void
jive_node_address_transform(jive::node * node, jive::memlayout_mapper * mapper);

void
jive_graph_address_transform(jive::graph * graph, jive::memlayout_mapper * mapper);

#endif
