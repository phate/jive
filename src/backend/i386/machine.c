#include <jive/backend/i386/machine.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/arch/transfer-instructions.h>
#include <jive/vsdg.h>

static size_t
jive_i386_create_copy(jive_region * region, jive_output * origin,
	jive_input ** xfer_in, jive_node * xfer_nodes[], jive_output ** xfer_out)
{
	jive_node * node = (jive_node *) jive_instruction_node_create(
		region, &jive_i386_instructions[jive_i386_int_transfer],
		&origin, NULL);
	
	*xfer_in = node->inputs[0];
	*xfer_out = node->outputs[0];
	xfer_nodes[0] = node;
	return 1;
}


const jive_transfer_instructions_factory jive_i386_transfer_instructions_factory = {
	.create_copy = jive_i386_create_copy,
	.max_nodes = 1
};

