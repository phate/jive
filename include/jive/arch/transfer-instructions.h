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
	
	size_t (*create_spill)(struct jive_region * region, struct jive_output * origin,
		struct jive_input **spill_in, struct jive_node * spill_nodes[], struct jive_node ** store_node);
		
	size_t (*create_restore)(struct jive_region * region, struct jive_output * stackslot,
		struct jive_node ** load_node, struct jive_node * restore_nodes[], struct jive_output ** restore_out);
	
	size_t max_nodes;
};

#endif
