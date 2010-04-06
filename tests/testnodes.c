#include "testnodes.h"

const jive_node_class TEST_NODE_CLASS = {
	0, "node", sizeof(test_node), 0,
};

const jive_value_class TEST_VALUE_CLASS = {
	0, "value", sizeof(test_value)
};

const jive_operand_class TEST_OPERAND_CLASS = {
	0, "operand", sizeof(test_operand),
};

static jive_operand_list *
test_input_list_create(jive_graph * graph, size_t ninputs, test_value * const inputs[], test_operand ** operands)
{
	*operands = 0;
	if (!ninputs) return 0;
	
	*operands = (test_operand *) jive_malloc(graph, sizeof(test_operand) * ninputs);
	
	size_t n;
	for(n=0; n<ninputs; n++) {
		(*operands)[n].type = &TEST_OPERAND_CLASS;
		(*operands)[n].value = inputs[n];
		(*operands)[n].index = n;
	}
	
	return (jive_operand_list *)(*operands);
}

jive_node *
test_node_create(jive_graph * graph, size_t ninputs,
	test_value * const inputs[])
{
	test_operand * operands;
	jive_operand_list * list = test_input_list_create(graph, ninputs, inputs, &operands);
	jive_node * _node = jive_node_create(graph,
		&TEST_NODE_CLASS,
		ninputs, list);
	test_node * node = (test_node *)_node;
	
	node->noperands = ninputs;
	node->operands = operands;
	
	node->value.type = &TEST_VALUE_CLASS;
	jive_value_init((jive_value *)&node->value, _node);
	
	return _node;
}

test_value *
test_node_value(jive_node * node)
{
	return &((test_node *)node)->value;
}

test_operand *
test_node_operand(jive_node * node, unsigned int index)
{
	return &((test_node *)node)->operands[index];
}
