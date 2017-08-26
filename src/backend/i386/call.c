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
#include <jive/vsdg/simple_node.h>
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
	auto address = dynamic_cast<jive::simple_output*>(node->input(0)->origin());
	if (auto op = dynamic_cast<const jive::address::label_to_address_op *>(
		&address->node()->operation())) {
		jive::immediate imm(0, op->label());
		auto tmp = jive_immediate_create(region, &imm);
		call_instr = jive::create_instruction(region, &jive::i386::instr_call::instance(), {tmp});
	} else if (auto op = dynamic_cast<const jive::address::label_to_bitstring_op *>(
		&address->node()->operation())) {
		jive::immediate imm(0, op->label());
		auto tmp = jive_immediate_create(region, &imm);
		call_instr = jive::create_instruction(region, &jive::i386::instr_call::instance(), {tmp});
	} else {
		/* FIXME: cast address to bitstring first */
		call_instr = jive::create_instruction(region, &jive::i386::instr_call_reg::instance(),
			{address});
	}
	
	/* mark caller-saved regs as clobbered */
	auto clobber_eax = call_instr->add_output(&jive_i386_regcls_gpr_eax);
	auto clobber_edx = call_instr->add_output(&jive_i386_regcls_gpr_edx);
	auto clobber_ecx = call_instr->add_output(&jive_i386_regcls_gpr_ecx);
	auto clobber_flags = call_instr->add_output(&jive_i386_regcls_flags);
	(void) clobber_edx;
	(void) clobber_ecx;
	(void) clobber_flags;
	
	/* FIXME: for certain types of return values, need to pass in
	a pointer to the return value area as first (hidden) parameter */
	size_t offset = 0;
	for (size_t n = 0; n < nargs; n++) {
		auto value = dynamic_cast<jive::simple_output*>(node->input(n+1)->origin());
		
		auto value_cls = value->port().rescls();
		if (value_cls == &jive_root_resource_class) {
			/* FIXME: assumes  int32 */
			value_cls = &jive_i386_regcls_gpr;
		}
		
		auto slot_cls = jive_callslot_class_get(4, 4, offset);
		
		offset += 4;
		
		jive::node * split = jive_splitnode_create(node->region(), value, value_cls, slot_cls);
		
		call_instr->add_input(slot_cls, split->output(0));
	}
	
	JIVE_DEBUG_ASSERT(op.nresults() <= 1);
	
	if (op.nresults() == 1) {
		/* FIXME: assumes  int32 */
		node->output(0)->replace(clobber_eax);
	}
	
	for (size_t n = node->noperands(); n < node->ninputs(); n++) {
		auto orig_input = dynamic_cast<jive::simple_input*>(node->input(n));
		if (orig_input->port().gate()) {
			call_instr->add_input(orig_input->port().gate(), orig_input->origin());
		} else {
			call_instr->add_input(orig_input->port().rescls(), orig_input->origin());
		}
	}
	for (size_t n = op.nresults(); n < node->noutputs(); n++) {
		auto orig_output = dynamic_cast<jive::simple_output*>(node->output(n));
		jive::output * new_output;
		if (orig_output->port().gate()) {
			new_output = call_instr->add_output(orig_output->port().gate());
		} else {
			new_output = call_instr->add_output(orig_output->port().rescls());
		}
		orig_output->replace(new_output);
	}
	
	return call_instr;
}
