#include <jive/backend/i386/machine.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/arch/transfer-instructions.h>
#include <jive/arch/stackframe.h>
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

static size_t
jive_i386_create_spill(jive_region * region, jive_output * origin,
	jive_input **spill_in, jive_node * spill_nodes[], jive_node ** store_node)
{
	jive_stackframe * stack = jive_region_get_stackframe(region);
	
	long displacement = 0;
	jive_node * node = (jive_node *) jive_instruction_node_create(
		region, &jive_i386_instructions[jive_i386_int_store32_disp],
		(jive_output *[]){(jive_output *) stack->stackptr, origin}, &displacement);
	
	jive_resource_assign_input(stack->stackptr->base.resource, node->inputs[0]);
	
	*spill_in = node->inputs[1];
	spill_nodes[0] = node;
	*store_node = node;
	return 1;
}

static size_t
jive_i386_create_restore(jive_region * region, jive_output * stackslot,
	jive_node ** load_node, jive_node * restore_nodes[], jive_output ** restore_out)
{
	jive_stackframe * stack = jive_region_get_stackframe(region);
	
	long displacement = 0;
	jive_node * node = (jive_node *) jive_instruction_node_create(
		region, &jive_i386_instructions[jive_i386_int_load32_disp],
		(jive_output *[]){(jive_output *) stack->stackptr}, &displacement);
	
	jive_resource_assign_input(stack->stackptr->base.resource, node->inputs[0]);
	
	*load_node = node;
	restore_nodes[0] = node;
	*restore_out = node->outputs[0];
	return 1;
}



const jive_transfer_instructions_factory jive_i386_transfer_instructions_factory = {
	.create_copy = jive_i386_create_copy,
	.create_spill = jive_i386_create_spill,
	.create_restore = jive_i386_create_restore,
	.max_nodes = 1
};

