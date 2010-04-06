#ifndef JIVE_INTERNAL_GATESTR_H
#define JIVE_INTERNAL_GATESTR_H

#include <jive/gate.h>

typedef struct _jive_output_gate_head jive_output_gate_head;
typedef struct _jive_input_gate_head jive_input_gate_head;

struct _jive_output_gate_head {
	/* linked list of gates per node */
	struct { jive_output_gate_head * prev, * next; } node;
	/* linked list of outputs per gate */
	struct { jive_output_gate_head * prev, * next; } gate;
};

struct _jive_input_gate_head {
	/* linked list of gates per node */
	struct { jive_input_gate_head * prev, * next; } node;
	/* linked list of inputs per gate */
	struct { jive_input_gate_head * prev, * next; } gate;
};

typedef struct _jive_gate_value_head {
	struct { jive_input_gate_head * first, * last; } inputs;
	struct { jive_output_gate_head * first, * last; } outputs;	
} jive_gate_value_head;

typedef struct _jive_node_gateinfo {
	struct { jive_input_gate_head * first, * last; } input;
	struct { jive_output_gate_head * first, * last; } output;
} jive_node_gateinfo;

static inline jive_output_gate_head *
head_of_output_gate(jive_value_bits * o)
{
	return ((jive_output_gate_head *)o)-1;
}

static inline jive_input_gate_head *
head_of_input_gate(jive_input_bits * i)
{
	return ((jive_input_gate_head *)i)-1;
}

static inline jive_gate_value_head *
head_of_gate_value(jive_gate_value * g)
{
	return ((jive_gate_value_head *)g)-1;
}

#endif
