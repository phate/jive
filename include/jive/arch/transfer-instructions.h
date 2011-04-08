#ifndef JIVE_ARCH_TRANSFER_INSTRUCTIONS_H
#define JIVE_ARCH_TRANSFER_INSTRUCTIONS_H

#include <stddef.h>

struct jive_region;
struct jive_output;
struct jive_input;
struct jive_node;

typedef struct jive_xfer_block jive_xfer_block;

typedef struct jive_transfer_instructions_factory jive_transfer_instructions_factory;

struct jive_xfer_block {
	jive_input * input;
	jive_node * node;
	jive_output * output;
};

struct jive_transfer_instructions_factory {
	jive_xfer_block (*create_xfer)(struct jive_region * region, struct jive_output * origin,
		const struct jive_resource_class * in_class, const struct jive_resource_class * out_class);
};

#endif
