/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/graph.h>
#include <jive/nodeclass.h>

extern const jive_node_class TEST_NODE_CLASS;
extern const jive_value_class TEST_VALUE_CLASS;
extern const jive_operand_class TEST_OPERAND_CLASS;

typedef struct test_value test_value;
typedef struct test_operand test_operand;
typedef struct test_node test_node;

struct test_value {
	JIVE_VALUE_COMMON_FIELDS
};

struct test_operand {
	JIVE_OPERAND_COMMON_FIELDS(test_value)
	unsigned int index;
};

struct test_node {
	jive_node base;
	test_value value;
	test_operand * operands;
	unsigned int noperands;
};

test_operand *
test_node_operand(jive_node * node, unsigned int index);

test_value *
test_node_value(jive_node * node);

jive_node *
test_node_create(jive_graph * graph, size_t ninputs,
	test_value * const inputs[]);
