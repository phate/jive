/*
 * Copyright 2010 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TESTS_TESTNODES_HPP
#define JIVE_TESTS_TESTNODES_HPP

#include <memory>
#include <vector>

#include <jive/rvsdg/binary.hpp>
#include <jive/rvsdg/node.hpp>
#include <jive/rvsdg/region.hpp>
#include <jive/rvsdg/simple-node.hpp>
#include <jive/rvsdg/structural-node.hpp>
#include <jive/rvsdg/unary.hpp>

namespace jive {
namespace test {

/* unary operation */

class unary_op final : public jive::unary_op {
public:
	virtual
	~unary_op() noexcept;

	inline
	unary_op(
		const jive::port & srcport,
		const jive::port & dstport) noexcept
	: jive::unary_op(srcport, dstport)
	{}

	virtual bool
	operator==(const jive::operation & other) const noexcept override;

	virtual jive_unop_reduction_path_t
	can_reduce_operand(
		const jive::output * operand) const noexcept override;

	virtual jive::output *
	reduce_operand(
		jive_unop_reduction_path_t path,
		jive::output * operand) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;

	static inline jive::node *
	create(
		jive::region * region,
		const jive::port & srcport,
		jive::output * operand,
		const jive::port & dstport)
	{
		return simple_node::create(region, unary_op(srcport, dstport), {operand});
	}

	static inline jive::output *
	create_normalized(
		const jive::port & srcport,
		jive::output * operand,
		const jive::port & dstport)
	{
		unary_op op(srcport, dstport);
		return simple_node::create_normalized(operand->region(), op, {operand})[0];
	}
};

static inline bool
is_unary_op(const jive::operation & op) noexcept
{
	return dynamic_cast<const unary_op*>(&op);
}

static inline bool
is_unary_node(const jive::node * node) noexcept
{
	return is<unary_op>(node);
}

/* binary operation */

class binary_op final : public jive::binary_op {
public:
	virtual
	~binary_op() noexcept;

	inline
	binary_op(
		const jive::port & srcport,
		const jive::port & dstport,
		const enum jive::binary_op::flags & flags) noexcept
	: jive::binary_op({srcport, srcport}, {dstport})
	, flags_(flags)
	{}

	virtual bool
	operator==(const jive::operation & other) const noexcept override;

	virtual jive_binop_reduction_path_t
	can_reduce_operand_pair(
		const jive::output * op1,
		const jive::output * op2) const noexcept override;

	virtual jive::output *
	reduce_operand_pair(
		jive_unop_reduction_path_t path,
		jive::output * op1,
		jive::output * op2) const override;

	virtual enum jive::binary_op::flags
	flags() const noexcept override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;

	static inline jive::node *
	create(
		const jive::port & srcport,
		const jive::port & dstport,
		jive::output * op1,
		jive::output * op2)
	{
		binary_op op(srcport, dstport, jive::binary_op::flags::none);
		return simple_node::create(op1->region(), op, {op1, op2});
	}

	static inline jive::output *
	create_normalized(
		const jive::port & srcport,
		const jive::port & dstport,
		jive::output * op1,
		jive::output * op2)
	{
		binary_op op(srcport, dstport, jive::binary_op::flags::none);
		return simple_node::create_normalized(op1->region(), op, {op1, op2})[0];
	}

private:
	enum jive::binary_op::flags flags_;
};

/* simple operation */

class simple_op final : public jive::simple_op {
public:
	virtual
	~simple_op() noexcept;

	inline
	simple_op(
		const std::vector<jive::port> & operands,
		const std::vector<jive::port> & results)
	: jive::simple_op(operands, results)
	{}

	simple_op(const simple_op &) = default;

	inline
	simple_op(jive::test::simple_op && other) = default;
	
	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

static inline jive::node *
simple_node_create(
	jive::region * region,
	const std::vector<jive::port> & iports,
	const std::vector<jive::output*> & operands,
	const std::vector<jive::port> & oports)
{
	return simple_node::create(region, simple_op(iports, oports), operands);
}

static inline std::vector<jive::output*>
simple_node_normalized_create(
	jive::region * r,
	const std::vector<jive::port> & iports,
	const std::vector<jive::output*> & operands,
	const std::vector<jive::port> & oports)
{
	jive::test::simple_op op(iports, oports);
	return simple_node::create_normalized(r, op, operands);
}

/* structural operation */

class structural_op final : public jive::structural_op {
public:
	virtual
	~structural_op() noexcept;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class structural_node final : public jive::structural_node {
public:
	~structural_node() override;

private:
	structural_node(
		jive::region * parent,
		size_t nsubregions)
	: jive::structural_node(structural_op(), parent, nsubregions)
	{}

public:
	static structural_node *
	create(
		jive::region * parent,
		size_t nsubregions)
	{
		return new structural_node(parent, nsubregions);
	}

	virtual structural_node *
	copy(jive::region * region, substitution_map & smap) const override;
};

}}

#endif
