/*
 * Copyright 2010 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TESTS_TESTNODES_H
#define JIVE_TESTS_TESTNODES_H

#include <memory>
#include <vector>

#include <jive/vsdg/node.h>

/* test node */

extern const jive_node_class JIVE_TEST_NODE;

class test_operation final : public jive::operation {
public:
	virtual
	~test_operation() noexcept;

	test_operation(
		size_t noperands, const jive::base::type * const operand_types[],
		size_t nresults, const jive::base::type * const result_types[]);

	test_operation(const test_operation & other);

	inline
	test_operation() noexcept {}

	inline
	test_operation(test_operation && other) noexcept = default;
	
	inline const std::vector<std::unique_ptr<jive::base::type>> &
	operand_types() const noexcept { return operand_types_; }

	inline const std::vector<std::unique_ptr<jive::base::type>> &
	result_types() const noexcept { return result_types_; }

private:
	std::vector<std::unique_ptr<jive::base::type>> operand_types_;
	std::vector<std::unique_ptr<jive::base::type>> result_types_;
};

typedef jive::operation_node<test_operation> jive_test_node;

jive_node *
jive_test_node_create(struct jive_region * region,
	size_t noperands, const jive::base::type * const operand_types[],
	jive::output * const operands[], size_t nresults,
	const jive::base::type * const result_types[]);

void
jive_test_node_create_normalized(struct jive_graph * graph, size_t noperands,
	const jive::base::type * const operand_types[], jive::output * const operands[],
	size_t nresults, const jive::base::type * const result_types[], jive::output * results[]);

JIVE_EXPORTED_INLINE const jive_test_node *
jive_test_node_const_cast(const jive_node * self)
{
	if (jive_node_isinstance(self, &JIVE_TEST_NODE))
		return (const jive_test_node *)self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_test_node *
jive_test_node_cast(jive_node * self)
{
	if (jive_node_isinstance(self, &JIVE_TEST_NODE))
		return (jive_test_node *)self;
	else
		return NULL;
}

#endif
