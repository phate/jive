#include <jive/backend/i386/machine.h>

#include <jive/arch/instruction.h>
#include <jive/arch/stackslot.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/registerset.h>
#include <jive/backend/i386/subroutine.h>
#include <jive/vsdg.h>

static jive_subroutine *
lookup_subroutine_by_region(jive_region * region)
{
	for (;;) {
		if (!region->anchor)
			return NULL;
		jive_subroutine_node * sub = jive_subroutine_node_cast(region->anchor->node);
		if (sub)
			return sub->attrs.subroutine;
		region = region->parent;
	}
}

static void
get_slot_memory_reference(const jive_i386_subroutine * subroutine, const jive_resource_class * rescls, 
	jive_immediate * displacement, jive_output ** base)
{
	if (jive_resource_class_isinstance(rescls, &JIVE_STACK_CALLSLOT_RESOURCE)) {
		jive_immediate_init(displacement, 0, &jive_label_spoffset, NULL, NULL);
		*base = subroutine->stackptr;
	} else {
		jive_immediate_init(displacement, 0, &jive_label_fpoffset, NULL, NULL);
		*base = subroutine->frameptr;
	}
}

jive_xfer_block
jive_i386_create_xfer(jive_region * region, jive_output * origin,
	const jive_resource_class * in_class, const jive_resource_class * out_class)
{
	jive_xfer_block xfer;
	
	jive_subroutine * subroutine_ = lookup_subroutine_by_region(region);
	jive_i386_subroutine * subroutine = (jive_i386_subroutine *) subroutine_;
	
	bool in_mem = !jive_resource_class_isinstance(in_class, &JIVE_REGISTER_RESOURCE);
	bool out_mem = !jive_resource_class_isinstance(out_class, &JIVE_REGISTER_RESOURCE);
	
	if (in_mem) {
		jive_immediate displacement;
		jive_output * base;
		get_slot_memory_reference(subroutine, in_class, &displacement, &base);
		xfer.node = jive_instruction_node_create_extended(
			region,
			&jive_i386_instructions[jive_i386_int_load32_disp],
			&base, &displacement);
		jive_input_auto_merge_variable(xfer.node->inputs[0]);
		xfer.input = jive_node_add_input(xfer.node, jive_resource_class_get_type(in_class), origin);
		xfer.input->required_rescls = in_class;
		xfer.output = xfer.node->outputs[0];
	} else if (out_mem) {
		jive_immediate displacement;
		jive_output * base;
		get_slot_memory_reference(subroutine, out_class, &displacement, &base);
		xfer.node = jive_instruction_node_create_extended(
			region,
			&jive_i386_instructions[jive_i386_int_store32_disp],
			(jive_output *[]){base, origin}, &displacement);
		jive_input_auto_merge_variable(xfer.node->inputs[0]);
		xfer.input = xfer.node->inputs[1];
		xfer.output = jive_node_add_output(xfer.node, jive_resource_class_get_type(out_class));
		xfer.output->required_rescls = out_class;
	} else {
		xfer.node = jive_instruction_node_create(
			region,
			&jive_i386_instructions[jive_i386_int_transfer],
			(jive_output *[]){origin}, NULL);
		xfer.input = xfer.node->inputs[0];
		xfer.output = xfer.node->outputs[0];
	}
	
	return xfer;
}

const jive_transfer_instructions_factory jive_i386_xfer_factory = {
	jive_i386_create_xfer
};

