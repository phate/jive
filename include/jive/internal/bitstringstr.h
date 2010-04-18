#ifndef JIVE_INTERNAL_BITSTRINGSTR_H
#define JIVE_INTERNAL_BITSTRINGSTR_H

#include <jive/bitstring.h>

typedef struct jive_bitstring_node jive_bitstring_node;

struct jive_bitstring_node {
	jive_node base;
	jive_value_bits output;
	jive_operand_bits * inputs;
	unsigned int ninputs;
};

struct _jive_bitsymbolicconstant {
	jive_bitstring_node base;
	char * name;
};

struct _jive_bitconstant {
	jive_bitstring_node base;
	jive_bitconstant_nodedata data;
};

struct _jive_bitslice {
	jive_bitstring_node base;
	jive_bitslice_nodedata data;
};

static inline const jive_bitstring_value_range *
jive_bitstring_input_value_range(const void * node, size_t index)
{
	return jive_value_bits_get_value_range( jive_bitstring_input(node, index) );
}

static inline jive_bitstring_value_range *
jive_bitstring_output_value_range(void * _node)
{
	jive_bitstring_node * node = _node;
	return &node->output._value_range;
}

jive_operand_list *
jive_input_bits_list_create(jive_graph * graph, size_t noperands, jive_value * const values[], jive_operand_bits ** operands);

static inline jive_value_bits *
jive_value_bits_cast(jive_value * value)
{
	if (value->type != &JIVE_VALUE_BITS)
		jive_graph_fatal_error(value->node->graph, "jive_value_bits expected");
	return (jive_value_bits *) value;
}

#endif
