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
