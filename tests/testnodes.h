/*
 * Copyright 2010 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TESTS_TESTNODES_H
#define JIVE_TESTS_TESTNODES_H

#include <memory>
#include <vector>

#include <jive/vsdg/node.h>
#include <jive/vsdg/simple.h>
#include <jive/vsdg/structural.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/simple_node.h>

namespace jive {
namespace test {

/* simple operation */

class simple_op final : public jive::simple_op {
public:
	virtual
	~simple_op() noexcept;

	simple_op(
		const std::vector<const jive::base::type*> & argument_types,
		const std::vector<const jive::base::type*> & result_types);

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
	const std::vector<const jive::base::type*> & operand_types,
	const std::vector<jive::output*> & operands,
	const std::vector<const jive::base::type*> & result_types)
{
	return region->add_simple_node(jive::test::simple_op(operand_types, result_types), operands);
}

static inline std::vector<jive::output*>
simple_node_normalized_create(
	jive::region * r,
	const std::vector<const jive::base::type*> & operand_types,
	const std::vector<jive::output*> & operands,
	const std::vector<const jive::base::type*> & result_types)
{
	jive::test::simple_op op(operand_types, result_types);
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
