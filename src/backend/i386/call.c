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
#include <jive/rvsdg/label.h>
#include <jive/rvsdg/region.h>
#include <jive/rvsdg/splitnode.h>

jive::node *
jive_i386_call_node_substitute(
	jive::node * node,
	const jive::call_op & op)
{
	jive::region * region = node->region();
	size_t nargs = node->ninputs() - 1;

	std::vector<jive::port> iports;
	std::vector<jive::port> oports;
	std::vector<jive::output*> operands;

	const jive::instruction * icls;
	auto & addrop = node->input(0)->origin()->node()->operation();
	if (auto op = dynamic_cast<const jive::lbl2addr_op*>(&addrop)) {
		icls = &jive::i386::instr_call::instance();
		jive::immediate imm(0, op->label());
		operands.push_back(jive::immediate_op::create(region, imm));
	} else if (auto op = dynamic_cast<const jive::label_to_bitstring_op*>(&addrop)) {
		icls = &jive::i386::instr_call::instance();
		jive::immediate imm(0, op->label());
		operands.push_back(jive::immediate_op::create(region, imm));
	} else {
		icls = &jive::i386::instr_call_reg::instance();
		operands.push_back(node->input(0)->origin());
	}

	/* FIXME: for certain types of return values, need to pass in
	a pointer to the return value area as first (hidden) parameter */
	for (size_t n = 0, offset = 0; n < nargs; n++) {
		auto value = node->input(n+1)->origin();
		auto value_cls = value->port().rescls();

		if (value_cls == &jive_root_resource_class) {
			/* FIXME: assumes  int32 */
			value_cls = &jive::i386::gpr_regcls;
		}
		
		auto slot_cls = jive_callslot_class_get(4, 4, offset);
		offset += 4;
		
		iports.push_back(slot_cls);
		operands.push_back(jive::split_op::create(value, value_cls, slot_cls));
	}

	oports.push_back(&jive::i386::eax_regcls);
	oports.push_back(&jive::i386::edx_regcls);
	oports.push_back(&jive::i386::ecx_regcls);
	oports.push_back(&jive::i386::cc_regcls);
	for (size_t n = op.nresults(); n < node->noutputs(); n++)
		oports.push_back(node->output(n)->port());

	auto call = jive::create_instruction(region, icls, operands, iports, oports);

	JIVE_DEBUG_ASSERT(op.nresults() <= 1);
	if (op.nresults() == 1) {
		/* FIXME: assumes  int32 */
		node->output(0)->replace(call->output(0));
	}

	for (size_t n = 4; n < node->noutputs(); n++)
		node->output(n)->replace(call->output(n));

	return call;
}
