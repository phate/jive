/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/subroutine/nodes.h>

#include <jive/arch/memorytype.h>
#include <jive/arch/subroutine.h>
#include <jive/rvsdg/controltype.h>
#include <jive/rvsdg/simple-node.h>

namespace jive {

subroutine_op::~subroutine_op() noexcept
{
}

size_t
subroutine_op::nresults() const noexcept
{
	return 1;
}

std::string
subroutine_op::debug_string() const
{
	return "SUBROUTINE";
}

std::unique_ptr<jive::operation>
subroutine_op::copy() const
{
	return std::unique_ptr<jive::operation>(new subroutine_op(*this));
}

simple_output *
subroutine_op::get_passthrough_enter_by_name(
	jive::region * region,
	const char * name) const noexcept
{
	/* FIXME: this function is broken */
	jive::node * enter = nullptr;
	JIVE_DEBUG_ASSERT(enter);
	for (size_t n = 0; n < enter->noutputs(); ++n) {
		auto o = enter->output(n);
		if (o->port().gate() && name == o->port().gate()->name())
			return dynamic_cast<simple_output*>(o);
	}
	return nullptr;
}

simple_output *
subroutine_op::get_passthrough_enter_by_index(jive::region * region, size_t index) const noexcept
{
	return get_passthrough_enter_by_name(region, signature().passthroughs[index].name.c_str());
}

jive::simple_input *
subroutine_op::get_passthrough_leave_by_name(
	jive::region * region,
	const char * name) const noexcept
{
	/* FIXME: this function is broken */
	jive::node * leave = nullptr;
	JIVE_DEBUG_ASSERT(leave);
	for (size_t n = 0; n < leave->ninputs(); ++n) {
		auto i = dynamic_cast<jive::simple_input*>(leave->input(n));
		if (i->port().gate() && name == i->port().gate()->name())
			return i;
	}
	return nullptr;
}

jive::simple_input *
subroutine_op::get_passthrough_leave_by_index(jive::region * region, size_t index) const noexcept
{
	return get_passthrough_leave_by_name(region, signature().passthroughs[index].name.c_str());
}

}
