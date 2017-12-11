/*
 * Copyright 2010 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TESTS_TESTNODES_H
#define JIVE_TESTS_TESTNODES_H

#include <memory>
#include <vector>

#include <jive/rvsdg/node.h>
#include <jive/rvsdg/region.h>
#include <jive/rvsdg/simple-node.h>
#include <jive/rvsdg/unary.h>

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
	: jive::unary_op()
	, srcport_(srcport)
	, dstport_(dstport)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual const jive::port &
	argument(size_t index) const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

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
		return region->add_simple_node(jive::test::unary_op(srcport, dstport), {operand});
	}

	static inline jive::output *
	create_normalized(
		const jive::port & srcport,
		jive::output * operand,
		const jive::port & dstport)
	{
		unary_op op(srcport, dstport);
		return jive::create_normalized(operand->region(), op, {operand})[0];
	}

private:
	jive::port srcport_;
	jive::port dstport_;
};

static inline bool
is_unary_op(const jive::operation & op) noexcept
{
	return dynamic_cast<const unary_op*>(&op);
}

static inline bool
is_unary_node(const jive::node * node) noexcept
{
	return is_opnode<unary_op>(node);
}

/* simple operation */

class simple_op final : public jive::simple_op {
public:
	virtual
	~simple_op() noexcept;

	inline
	simple_op(
		const std::vector<jive::port> & arguments,
		const std::vector<jive::port> & results)
	: jive::simple_op()
	, results_(results)
	, arguments_(arguments)
	{}

	inline
	simple_op() noexcept {}

	simple_op(const simple_op &) = default;

	inline
	simple_op(jive::test::simple_op && other) noexcept = default;
	
	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual size_t
	narguments() const noexcept override;

	virtual const jive::port &
	argument(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	std::vector<jive::port> results_;
	std::vector<jive::port> arguments_;
};

static inline jive::node *
simple_node_create(
	jive::region * region,
	const std::vector<jive::port> & iports,
	const std::vector<jive::output*> & operands,
	const std::vector<jive::port> & oports)
{
	return region->add_simple_node(jive::test::simple_op(iports, oports), operands);
}

static inline std::vector<jive::output*>
simple_node_normalized_create(
	jive::region * r,
	const std::vector<jive::port> & iports,
	const std::vector<jive::output*> & operands,
	const std::vector<jive::port> & oports)
{
	jive::test::simple_op op(iports, oports);
	return jive::create_normalized(r, op, operands);
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

static inline jive::structural_node *
structural_node_create(jive::region * parent, size_t nsubregions)
{
	return parent->add_structural_node(jive::test::structural_op(), nsubregions);
}

}}

#endif
