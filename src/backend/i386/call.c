#include <jive/backend/i386/call.h>

#include <jive/arch/address.h>
#include <jive/arch/call.h>
#include <jive/arch/instruction.h>
#include <jive/arch/stackslot.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/registerset.h>
#include <jive/bitstring/type.h>
#include <jive/regalloc/auxnodes.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/region.h>

jive_node *
jive_i386_call_node_substitute(jive_call_node * node)
{
	jive_region * region = node->base.region;
	
	size_t nargs = node->base.noperands - 1;
	
	/* distinguish between call to fixed address and register-indirect call */
	jive_node * call_instr;
	jive_output * address = node->base.inputs[0]->origin;
	if (address->node->class_ == &JIVE_LABEL_TO_ADDRESS_NODE) {
		jive_label_to_address_node * label_node = jive_label_to_address_node_cast(address->node);
		jive_immediate imm;
		jive_immediate_init(&imm, 0, label_node->attrs.label, NULL, NULL);
		call_instr = jive_instruction_node_create_extended(
			region,
			&jive_i386_instructions[jive_i386_call],
			0, &imm);
	} else if (address->node->class_ == &JIVE_LABEL_TO_BITSTRING_NODE) {
		jive_label_to_bitstring_node * label_node = jive_label_to_bitstring_node_cast(address->node);
		jive_immediate imm;
		jive_immediate_init(&imm, 0, label_node->attrs.label, NULL, NULL);
		call_instr = jive_instruction_node_create_extended(
			region,
			&jive_i386_instructions[jive_i386_call],
			0, &imm);
	} else {
		/* FIXME: cast address to bitstring first */
		call_instr = jive_instruction_node_create(
			region,
			&jive_i386_instructions[jive_i386_call_reg],
			(jive_output * []){address}, NULL);
	}
	
	/* mark caller-saved regs as clobbered */
	jive_output * clobber_eax = jive_node_add_constrained_output(call_instr, jive_i386_cls_eax);
	jive_output * clobber_edx = jive_node_add_constrained_output(call_instr, jive_i386_cls_edx);
	jive_output * clobber_ecx = jive_node_add_constrained_output(call_instr, jive_i386_cls_ecx);
	jive_output * clobber_flags = jive_node_add_constrained_output(call_instr, jive_i386_cls_flags);
	(void) clobber_edx;
	(void) clobber_ecx;
	(void) clobber_flags;
	
	/* FIXME: for certain types of return values, need to pass in
	a pointer to the return value area as first (hidden) parameter */
	size_t n, offset = 0;
	for (n = 0; n < nargs; n++) {
		jive_output * value = node->base.inputs[n + 1]->origin;
		
		const jive_resource_class * value_cls = value->required_rescls;
		const jive_type * value_type = jive_output_get_type(value);
		if (value_cls == &jive_root_resource_class) {
			/* FIXME: assumes  int32 */
			value_cls = jive_i386_cls_gpr;
		}
		
		const jive_resource_class * slot_cls = jive_callslot_class_get(4, 4, offset);
		const jive_type * slot_type = jive_resource_class_get_type(slot_cls);
		
		offset += 4;
		
		jive_node * split = jive_aux_split_node_create(
			node->base.region,
			value_type, value, value_cls,
			slot_type, slot_cls);
		
		jive_input * input = jive_node_add_input(call_instr, slot_type, split->outputs[0]);
		input->required_rescls = slot_cls;
	}
	
	JIVE_DEBUG_ASSERT(node->attrs.nreturns <= 1);
	
	if (node->attrs.nreturns == 1) {
		/* FIXME: assumes  int32 */
		jive_output_replace(node->base.outputs[0], clobber_eax);
	}
	
	for (n = node->base.noperands; n < node->base.ninputs; n++) {
		jive_input * orig_input = node->base.inputs[n];
		if (orig_input->gate) {
			jive_node_gate_input(call_instr, orig_input->gate, orig_input->origin);
		} else {
			jive_input * new_input = jive_node_add_input(call_instr, jive_input_get_type(orig_input), orig_input->origin);
			new_input->required_rescls = orig_input->required_rescls;
		}
	}
	for (n = node->attrs.nreturns; n < node->base.noutputs; n++) {
		jive_output * orig_output = node->base.outputs[n];
		jive_output * new_output;
		if (orig_output->gate) {
			new_output = jive_node_gate_output(call_instr, orig_output->gate);
		} else {
			new_output = jive_node_add_output(call_instr, jive_output_get_type(orig_output));
			new_output->required_rescls = orig_output->required_rescls;
		}
		jive_output_replace(orig_output, new_output);
	}
	
	return call_instr;
}
