#ifndef JIVE_INTERNAL_PASSTHROUGHSTR_H
#define JIVE_INTERNAL_PASSTHROUGHSTR_H

#include <jive/passthrough.h>

typedef struct _jive_output_passthrough_head jive_output_passthrough_head;
typedef struct _jive_input_passthrough_head jive_input_passthrough_head;

struct _jive_output_passthrough_head {
	/* linked list of gates per node */
	struct { jive_output_passthrough_head * prev, * next; } node;
	/* linked list of outputs per gate */
	struct { jive_output_passthrough_head * prev, * next; } gate;
};

struct _jive_input_passthrough_head {
	/* linked list of gates per node */
	struct { jive_input_passthrough_head * prev, * next; } node;
	/* linked list of inputs per gate */
	struct { jive_input_passthrough_head * prev, * next; } gate;
};

typedef struct _jive_passthrough_head {
	struct { jive_input_passthrough_head * first, * last; } inputs;
	struct { jive_output_passthrough_head * first, * last; } outputs;	
} jive_passthrough_head;

typedef struct _jive_node_passthrough_info {
	struct { jive_input_passthrough_head * first, * last; } input;
	struct { jive_output_passthrough_head * first, * last; } output;
} jive_node_passthrough_info;

static inline jive_output_passthrough_head *
head_of_output_passthrough(jive_value_bits * o)
{
	return ((jive_output_passthrough_head *)o)-1;
}

static inline jive_input_passthrough_head *
head_of_input_passthrough(jive_operand_bits * i)
{
	return ((jive_input_passthrough_head *)i)-1;
}

static inline jive_passthrough_head *
head_of_passthrough(jive_passthrough * g)
{
	return ((jive_passthrough_head *)g)-1;
}

#endif
