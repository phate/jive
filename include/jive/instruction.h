#ifndef JIVE_INSTRUCTION_H
#define JIVE_INSTRUCTION_H

#include <jive/bitstring.h>
#include <jive/buffer.h>

#include <jive/types.h>

typedef struct jive_instruction jive_instruction;

extern const jive_node_class JIVE_INSTRUCTION;
extern const jive_value_class JIVE_OUTPUTREG;

typedef struct jive_instruction_sequence jive_instruction_sequence;

jive_node *
jive_instruction_create(jive_graph * graph,
	const jive_instruction_class * icls,
	jive_value * const inputs[],
	const long immediates[]);

jive_operand_bits *
jive_instruction_input(const void * node, unsigned int index);

jive_value *
jive_instruction_output(const void * node, unsigned int index);

jive_cpureg_t
jive_instruction_inputreg(const void * node, unsigned int index);

jive_cpureg_t
jive_instruction_outputreg(const void * node, unsigned int index);

long
jive_instruction_immediate(const void * node, unsigned int index);

const jive_instruction_class *
jive_instruction_get_class(const jive_node * node);

void
jive_instruction_swap_inputs(jive_node * node);

void
jive_instruction_use_stackslot(jive_node * node, jive_stackslot * slot);

jive_stackslot *
jive_instruction_get_stackslot(const jive_node * node);

extern jive_instruction_class jive_pseudo_nop;

/* instruction sequences */

void
jive_instruction_sequence_init(jive_instruction_sequence * seq);

void
jive_graph_sequentialize(jive_graph * graph, jive_instruction_sequence * seq);

void
jive_instruction_sequence_append(jive_instruction_sequence * seq, jive_instruction * instruction);

bool
jive_instruction_sequence_encode(const jive_instruction_sequence * seq, jive_buffer * buffer, const jive_machine * machine);

void *
jive_buffer_map_executable(jive_buffer * buffer);

/* public structures */

struct jive_instruction_sequence {
	jive_instruction * first;
	jive_instruction * last;
};


#endif
