#ifndef JIVE_PASSTHROUGH_H
#define JIVE_PASSTHROUGH_H

#include <jive/instruction.h>

jive_passthrough *
jive_passthrough_create(jive_graph * graph, unsigned int nbits, jive_cpureg_class_t regcls, const char * description);

jive_value *
jive_output_passthrough_create(jive_node * node, jive_passthrough * passthrough);

jive_operand_bits *
jive_input_passthrough_create(jive_node * node, jive_passthrough * passthrough, jive_value * value);

/* public structures */

struct _jive_passthrough {
	unsigned int nbits;
	jive_cpureg_class_t regcls;
	const char * description;
};

#endif
