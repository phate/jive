/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/compilate.h>
#include <jive/arch/immediate-node.h>
#include <jive/arch/immediate-type.h>
#include <jive/arch/instruction.h>
#include <jive/arch/label-mapper.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/simple_node.h>
#include <jive/vsdg/traverser.h>

#include <stdio.h>
#include <string.h>

namespace jive {

instruction_op::~instruction_op() noexcept
{
}

bool
instruction_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const instruction_op *>(&other);
	return op
	    && op->icls() == icls()
	    && op->results_ == results_
	    && op->arguments_ == arguments_;
}

size_t
instruction_op::narguments() const noexcept
{
	return arguments_.size();
}

const jive::base::type &
instruction_op::argument_type(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return arguments_[index].type();
}

const jive::port &
instruction_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return arguments_[index];
}

size_t
instruction_op::nresults() const noexcept
{
	return results_.size();
}

const jive::base::type &
instruction_op::result_type(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return results_[index].type();
}

const jive::port &
instruction_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return results_[index];
}

std::string
instruction_op::debug_string() const
{
	return icls()->name();
}

std::unique_ptr<jive::operation>
instruction_op::copy() const
{
	return std::unique_ptr<jive::operation>(new instruction_op(*this));
}

}

jive::node *
jive_instruction_node_create_simple(
	jive::region * region,
	const jive::instruction_class * icls,
	jive::output * const * operands,
	const int64_t * immediates)
{
	std::vector<jive::immediate> imm;
	for (size_t n = 0; n < icls->nimmediates(); ++n)
		imm.push_back(jive::immediate(immediates[0]));

	return jive_instruction_node_create_extended(region, icls, operands, &imm[0]);
}

jive::node *
jive_instruction_node_create_extended(
	jive::region * region,
	const jive::instruction_class * icls,
	jive::output * const * operands_,
	const jive::immediate immediates_[])
{
	std::vector<jive::output*> operands(operands_, operands_ + icls->ninputs());
	std::vector<jive::immediate> immediates(immediates_, immediates_ + icls->nimmediates());

	return jive_instruction_node_create(region, icls, operands, immediates, {}, {}, {});
}

jive::node *
jive_instruction_node_create(
	jive::region * region,
	const jive::instruction_class * icls,
	const std::vector<jive::output*> & operands,
	const std::vector<jive::immediate> & immediates,
	const std::vector<const jive::state::type*> & itypes,
	const std::vector<jive::output*> & istates,
	const std::vector<const jive::state::type*> & otypes)
{
	std::vector<jive::output*> arguments(operands.begin(), operands.end());
	for (size_t n = 0; n < icls->nimmediates(); n++)
		arguments.push_back(jive_immediate_create(region, &immediates[n]));
	arguments.insert(arguments.end(), istates.begin(), istates.end());

	jive::instruction_op op(icls, itypes, otypes);
	return region->add_simple_node(op, arguments);
}
