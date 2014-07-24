/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/subroutine/nodes.h>

#include <jive/arch/memorytype.h>
#include <jive/arch/subroutine.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node-private.h>

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
	static const ctl::type type;
	return type;
}

jive_node *
subroutine_head_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	JIVE_DEBUG_ASSERT(!region->top);
	jive_node * node =jive_opnode_create(
		*this,
		&JIVE_SUBROUTINE_ENTER_NODE,
		region,
		arguments, arguments + narguments);
	static_cast<jive::ctl::output*>(node->outputs[0])->set_active(false);
	region->top = node;
	return node;
}

std::string
subroutine_head_op::debug_string() const
{
	return "SUBROUTINE_HEAD";
}

subroutine_tail_op::~subroutine_tail_op() noexcept
{
}

size_t
subroutine_tail_op::narguments() const noexcept
{
	return 1;
}

const base::type &
subroutine_tail_op::argument_type(size_t index) const noexcept
{
	static const ctl::type type;
	return type;
}

jive_node *
subroutine_tail_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	JIVE_DEBUG_ASSERT(!region->bottom);
	jive_node * node = jive_opnode_create(
		*this,
		&JIVE_SUBROUTINE_LEAVE_NODE,
		region,
		arguments, arguments + narguments);
	region->bottom = node;
	return node;
}

std::string
subroutine_tail_op::debug_string() const
{
	return "SUBROUTINE_TAIL";
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

jive_node *
subroutine_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return jive_opnode_create(
		*this,
		&JIVE_SUBROUTINE_NODE,
		region,
		arguments, arguments + narguments);
}

std::string
subroutine_op::debug_string() const
{
	return "SUBROUTINE";
}

output *
subroutine_op::get_passthrough_enter_by_name(jive_region * region, const char * name) const noexcept
{
	jive_node * enter = region->top;
	JIVE_DEBUG_ASSERT(enter);
	for (size_t n = 0; n < enter->noutputs; ++n) {
		output * o = enter->outputs[n];
		if (o->gate && strcmp(name, o->gate->name) == 0) {
			return o;
		}
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
	jive_node * leave = region->bottom;
	JIVE_DEBUG_ASSERT(leave);
	for (size_t n = 0; n < leave->ninputs; ++n) {
		input * i = leave->inputs[n];
		if (i->gate && strcmp(name, i->gate->name) == 0) {
			return i;
		}
	}
	return nullptr;
}

input *
subroutine_op::get_passthrough_leave_by_index(jive_region * region, size_t index) const noexcept
{
	return get_passthrough_leave_by_name(region, signature().passthroughs[index].name.c_str());
}

}

const jive_node_class JIVE_SUBROUTINE_ENTER_NODE = {
	parent : &JIVE_NODE,
	name : "SUBROUTINE_ENTER",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

const jive_node_class JIVE_SUBROUTINE_LEAVE_NODE = {
	parent : &JIVE_NODE,
	name : "SUBROUTINE_LEAVE",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

const jive_node_class JIVE_SUBROUTINE_NODE = {
	parent : &JIVE_NODE,
	name : "SUBROUTINE",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};
