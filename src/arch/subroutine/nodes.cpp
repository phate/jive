/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/subroutine/nodes.hpp>

#include <jive/arch/addresstype.hpp>
#include <jive/arch/subroutine.hpp>
#include <jive/rvsdg/control.hpp>
#include <jive/rvsdg/simple-node.hpp>

namespace jive {

subroutine_op::~subroutine_op() noexcept
{
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
	JIVE_ASSERT(0);
	#if 0
	jive::node * enter = nullptr;
	JIVE_DEBUG_ASSERT(enter);
	for (size_t n = 0; n < enter->noutputs(); ++n) {
		auto o = enter->output(n);
		if (o->port().gate() && name == o->port().gate()->name())
			return dynamic_cast<simple_output*>(o);
	}
	#endif
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
	JIVE_ASSERT(0);
	#if 0
	/* FIXME: this function is broken */
	jive::node * leave = nullptr;
	JIVE_DEBUG_ASSERT(leave);
	for (size_t n = 0; n < leave->ninputs(); ++n) {
		auto i = dynamic_cast<jive::simple_input*>(leave->input(n));
		if (i->port().gate() && name == i->port().gate()->name())
			return i;
	}
	#endif
	return nullptr;
}

jive::simple_input *
subroutine_op::get_passthrough_leave_by_index(jive::region * region, size_t index) const noexcept
{
	return get_passthrough_leave_by_name(region, signature().passthroughs[index].name.c_str());
}

}
