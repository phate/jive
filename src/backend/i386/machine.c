/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/backend/i386/machine.h>

#include <jive/arch/instruction.h>
#include <jive/arch/instructionset.h>
#include <jive/arch/stackslot.h>
#include <jive/arch/subroutine/nodes.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/registerset.h>
#include <jive/backend/i386/subroutine.h>
#include <jive/vsdg.h>

static void
get_slot_memory_reference(const jive::resource_class * rescls,
	jive::immediate * displacement, jive::output ** base,
	jive::output * sp, jive::output * fp)
{
	if (jive_resource_class_isinstance(rescls, &JIVE_STACK_CALLSLOT_RESOURCE)) {
		*displacement = jive::immediate(0, &jive_label_spoffset);
		*base = sp;
	} else {
		*displacement = jive::immediate(0, &jive_label_fpoffset);
		*base = fp;
	}
}

jive_xfer_description
jive_i386_create_xfer(jive::region * region, jive::simple_output * origin,
	const jive::resource_class * in_class, const jive::resource_class * out_class)
{
	jive_xfer_description xfer;
	
	jive::node * sub = jive_region_get_subroutine_node(region);
	
	auto sp = jive_subroutine_node_get_sp(sub);
	auto fp = jive_subroutine_node_get_fp(sub);
	
	bool in_mem = !jive_resource_class_isinstance(in_class, &JIVE_REGISTER_RESOURCE);
	bool out_mem = !jive_resource_class_isinstance(out_class, &JIVE_REGISTER_RESOURCE);
	
	if (in_mem) {
		jive::output * base;
		jive::immediate displacement;
		get_slot_memory_reference(in_class, &displacement, &base, sp, fp);
		auto imm = jive_immediate_create(region, &displacement);
		xfer.node = jive::create_instruction(region, &jive::i386::instr_int_load32_disp::instance(),
			{base, imm});
		xfer.input = dynamic_cast<jive::simple_input*>(xfer.node->add_input(in_class, origin));
		xfer.output = dynamic_cast<jive::simple_output*>(xfer.node->output(0));
	} else if (out_mem) {
		jive::output * base;
		jive::immediate displacement;
		get_slot_memory_reference(out_class, &displacement, &base, sp, fp);
		auto imm = jive_immediate_create(region, &displacement);
		xfer.node = jive::create_instruction(region, &jive::i386::instr_int_store32_disp::instance(),
			{base, origin, imm});
		xfer.input = dynamic_cast<jive::simple_input*>(xfer.node->input(1));
		xfer.output = dynamic_cast<jive::simple_output*>(xfer.node->add_output(out_class));
	} else {
		xfer.node = jive::create_instruction(region, &jive::i386::instr_int_transfer::instance(),
			{origin});
		xfer.input = dynamic_cast<jive::simple_input*>(xfer.node->input(0));
		xfer.output = dynamic_cast<jive::simple_output*>(xfer.node->output(0));
	}
	
	return xfer;
}
