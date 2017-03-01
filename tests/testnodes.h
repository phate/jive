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
#include <jive/vsdg/operators/simple.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/simple_node.h>

namespace jive {
namespace test {

class op final : public jive::simple_op {
public:
	virtual
	~op() noexcept;

	op(
		const std::vector<const jive::base::type*> & argument_types,
		const std::vector<const jive::base::type*> & result_types);

	op(const jive::test::op & other);

	inline
	op() noexcept {}

	inline
	op(jive::test::op && other) noexcept = default;
	
	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual size_t
	narguments() const noexcept override;

	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	std::vector<std::unique_ptr<const jive::base::type>> argument_types_;
	std::vector<std::unique_ptr<const jive::base::type>> result_types_;
};

static inline jive::node *
node_create(
	jive::region * region,
	const std::vector<const jive::base::type*> & operand_types,
	const std::vector<jive::oport*> & operands,
	const std::vector<const jive::base::type*> & result_types)
{
	return region->add_simple_node(jive::test::op(operand_types, result_types), operands);
}

static inline std::vector<jive::oport*>
node_normalized_create(
	jive::region * r,
	const std::vector<const jive::base::type*> & operand_types,
	const std::vector<jive::oport*> & operands,
	const std::vector<const jive::base::type*> & result_types)
{
	return jive_node_create_normalized(r, jive::test::op(operand_types, result_types), operands);
}

}}

#endif
