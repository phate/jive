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
get_slot_memory_reference(const jive_resource_class * rescls,
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
jive_i386_create_xfer(jive::region * region, jive::output * origin,
	const jive_resource_class * in_class, const jive_resource_class * out_class)
{
	jive_xfer_description xfer;
	
	jive::node * sub = jive_region_get_subroutine_node(region);
	
	jive::output * sp = jive_subroutine_node_get_sp(sub);
	jive::output * fp = jive_subroutine_node_get_fp(sub);
	
	bool in_mem = !jive_resource_class_isinstance(in_class, &JIVE_REGISTER_RESOURCE);
	bool out_mem = !jive_resource_class_isinstance(out_class, &JIVE_REGISTER_RESOURCE);
	
	if (in_mem) {
		jive::immediate displacement;
		jive::output * base;
		get_slot_memory_reference(in_class, &displacement, &base, sp, fp);
		xfer.node = jive_instruction_node_create_extended(
			region,
			&jive_i386_instr_int_load32_disp,
			&base, &displacement);
		xfer.input = xfer.node->add_input(in_class, origin);
		xfer.output = xfer.node->output(0);
	} else if (out_mem) {
		jive::immediate displacement;
		jive::output * base;
		get_slot_memory_reference(out_class, &displacement, &base, sp, fp);
		jive::output * tmparray0[] = {base, origin};
		xfer.node = jive_instruction_node_create_extended(
			region,
			&jive_i386_instr_int_store32_disp,
			tmparray0, &displacement);
		xfer.input = xfer.node->input(1);
		xfer.output = xfer.node->add_output(out_class);
	} else {
		jive::output * tmparray1[] = {origin};
		xfer.node = jive_instruction_node_create(
			region,
			&jive_i386_instr_int_transfer,
			tmparray1, NULL);
		xfer.input = xfer.node->input(0);
		xfer.output = xfer.node->output(0);
	}
	
	return xfer;
}
