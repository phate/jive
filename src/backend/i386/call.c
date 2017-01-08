/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/backend/i386/call.h>

#include <jive/arch/address.h>
#include <jive/arch/call.h>
#include <jive/arch/instruction.h>
#include <jive/arch/stackslot.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/registerset.h>
#include <jive/types/bitstring/type.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/splitnode.h>

jive::node *
jive_i386_call_node_substitute(
	jive::node * node,
	const jive::call_operation & op)
{
	jive::region * region = node->region();
	
	size_t nargs = node->noperands() - 1;
	
	/* distinguish between call to fixed address and register-indirect call */
	jive::node * call_instr;
	jive::output * address = dynamic_cast<jive::output*>(node->input(0)->origin());
	if (auto op = dynamic_cast<const jive::address::label_to_address_op *>(
		&address->node()->operation())) {
		jive::immediate imm(0, op->label());
		call_instr = jive_instruction_node_create_extended(
			region,
			&jive_i386_instr_call,
			0, &imm);
	} else if (auto op = dynamic_cast<const jive::address::label_to_bitstring_op *>(
		&address->node()->operation())) {
		jive::immediate imm(0, op->label());
		call_instr = jive_instruction_node_create_extended(
			region,
			&jive_i386_instr_call,
			0, &imm);
	} else {
		jive::output *  tmparray0[] = {address};
		/* FIXME: cast address to bitstring first */
		call_instr = jive_instruction_node_create(
			region,
			&jive_i386_instr_call_reg,
			tmparray0, NULL);
	}
	
	/* mark caller-saved regs as clobbered */
	jive::output * clobber_eax = dynamic_cast<jive::output*>(
		call_instr->add_output(&jive_i386_regcls_gpr_eax.base));
	jive::output * clobber_edx = dynamic_cast<jive::output*>(
		call_instr->add_output(&jive_i386_regcls_gpr_edx.base));
	jive::output * clobber_ecx = dynamic_cast<jive::output*>(
		call_instr->add_output(&jive_i386_regcls_gpr_ecx.base));
	jive::output * clobber_flags = dynamic_cast<jive::output*>(
		call_instr->add_output(&jive_i386_regcls_flags.base));
	(void) clobber_edx;
	(void) clobber_ecx;
	(void) clobber_flags;
	
	/* FIXME: for certain types of return values, need to pass in
	a pointer to the return value area as first (hidden) parameter */
	size_t offset = 0;
	for (size_t n = 0; n < nargs; n++) {
		jive::output * value = dynamic_cast<jive::output*>(node->input(n+1)->origin());
		
		const jive_resource_class * value_cls = value->rescls();
		const jive::base::type * value_type = &value->type();
		if (value_cls == &jive_root_resource_class) {
			/* FIXME: assumes  int32 */
			value_cls = &jive_i386_regcls_gpr.base;
		}
		
		const jive_resource_class * slot_cls = jive_callslot_class_get(4, 4, offset);
		const jive::base::type * slot_type = jive_resource_class_get_type(slot_cls);
		
		offset += 4;
		
		jive::node * split = jive_splitnode_create(node->region(), value_type, value, value_cls,
			slot_type, slot_cls);
		
		call_instr->add_input(slot_cls, split->output(0));
	}
	
	JIVE_DEBUG_ASSERT(op.result_types().size() <= 1);
	
	if (op.result_types().size() == 1) {
		/* FIXME: assumes  int32 */
		node->output(0)->replace(clobber_eax);
	}
	
	for (size_t n = node->noperands(); n < node->ninputs(); n++) {
		jive::input * orig_input = dynamic_cast<jive::input*>(node->input(n));
		if (orig_input->gate()) {
			call_instr->add_input(orig_input->gate(), orig_input->origin());
		} else {
			call_instr->add_input(orig_input->rescls(), orig_input->origin());
		}
	}
	for (size_t n = op.result_types().size(); n < node->noutputs(); n++) {
		jive::output * orig_output = dynamic_cast<jive::output*>(node->output(n));
		jive::output * new_output;
		if (orig_output->gate()) {
			new_output = dynamic_cast<jive::output*>(call_instr->add_output(orig_output->gate()));
		} else {
			new_output = dynamic_cast<jive::output*>(call_instr->add_output(orig_output->rescls()));
		}
		orig_output->replace(new_output);
	}
	
	return call_instr;
}
