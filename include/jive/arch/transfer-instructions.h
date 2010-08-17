#ifndef JIVE_ARCH_TRANSFER_INSTRUCTIONS_H
#define JIVE_ARCH_TRANSFER_INSTRUCTIONS_H

#include <stddef.h>

struct jive_region;
struct jive_output;
struct jive_input;
struct jive_node;

typedef struct jive_transfer_instructions_factory jive_transfer_instructions_factory;

struct jive_transfer_instructions_factory {
	size_t (*create_copy)(struct jive_region * region, struct jive_output * origin,
		struct jive_input ** xfer_in, struct jive_node * xfer_nodes[], struct jive_output ** xfer_out);
	
	size_t max_nodes;
};

#endif
