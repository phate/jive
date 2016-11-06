/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/subroutine/nodes.h>

#include <jive/arch/memorytype.h>
#include <jive/arch/subroutine.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/seqtype.h>

namespace jive {

subroutine_head_op::~subroutine_head_op() noexcept
{
}

size_t
subroutine_head_op::nresults() const noexcept
{
	return 1;
}

const base::type &
subroutine_head_op::result_type(size_t index) const noexcept
{
	return seq::seqtype;
}
std::string
subroutine_head_op::debug_string() const
{
	return "SUBROUTINE_HEAD";
}

std::unique_ptr<jive::operation>
subroutine_head_op::copy() const
{
	return std::unique_ptr<jive::operation>(new subroutine_head_op(*this));
}

subroutine_tail_op::~subroutine_tail_op() noexcept
{
}

size_t
subroutine_tail_op::narguments() const noexcept
{
	return 2;
}

const base::type &
subroutine_tail_op::argument_type(size_t index) const noexcept
{
	if (index == 0)
		return seq::seqtype;
	else {
		return jive::ctl::boolean;
	}
}

std::string
subroutine_tail_op::debug_string() const
{
	return "SUBROUTINE_TAIL";
}

std::unique_ptr<jive::operation>
subroutine_tail_op::copy() const
{
	return std::unique_ptr<jive::operation>(new subroutine_tail_op(*this));
}

subroutine_op::~subroutine_op() noexcept
{
}

size_t
subroutine_op::nresults() const noexcept
{
	return 1;
}

const base::type &
subroutine_op::result_type(size_t index) const noexcept
{
	/* FIXME: don't use memory type here */
	static const jive::mem::type objstate_type;
	return objstate_type;
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

output *
subroutine_op::get_passthrough_enter_by_name(jive_region * region, const char * name) const noexcept
{
	jive_node * enter = region->top();
	JIVE_DEBUG_ASSERT(enter);
	for (size_t n = 0; n < enter->noutputs(); ++n) {
		output * o = enter->output(n);
		if (o->gate() && name == o->gate()->name())
			return o;
	}
	return nullptr;
}

output *
subroutine_op::get_passthrough_enter_by_index(jive_region * region, size_t index) const noexcept
{
	return get_passthrough_enter_by_name(region, signature().passthroughs[index].name.c_str());
}

input *
subroutine_op::get_passthrough_leave_by_name(jive_region * region, const char * name) const noexcept
{
	jive_node * leave = region->bottom();
	JIVE_DEBUG_ASSERT(leave);
	for (size_t n = 0; n < leave->ninputs(); ++n) {
		input * i = leave->input(n);
		if (i->gate() && name == i->gate()->name())
			return i;
	}
	return nullptr;
}

input *
subroutine_op::get_passthrough_leave_by_index(jive_region * region, size_t index) const noexcept
{
	return get_passthrough_leave_by_name(region, signature().passthroughs[index].name.c_str());
}

}
