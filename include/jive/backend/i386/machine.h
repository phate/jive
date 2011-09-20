#ifndef JIVE_BACKEND_I386_MACHINE_H
#define JIVE_BACKEND_I386_MACHINE_H

#include <jive/arch/transfer-instructions.h>

extern const jive_transfer_instructions_factory jive_i386_xfer_factory;

jive_xfer_block
jive_i386_create_xfer(struct jive_region * region, struct jive_output * origin,
	const struct jive_resource_class * in_class, const struct jive_resource_class * out_class);

#endif
