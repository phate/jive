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

subroutine_op::subroutine_op(const subroutine_op & other)
	: subroutine_(other.subroutine_) // FIXME: this is wrong, need to copy
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


}

jive_subroutine_deprecated *
jive_subroutine_copy(const jive_subroutine_deprecated * self,
	jive_node * new_enter_node, jive_node * new_leave_node);

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

static void
jive_subroutine_node_fini_(jive_node * self_);

const jive_node_class JIVE_SUBROUTINE_NODE = {
	parent : &JIVE_NODE,
	name : "SUBROUTINE",
	fini : jive_subroutine_node_fini_, /* override */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

static void
jive_subroutine_node_fini_(jive_node * self_)
{
	jive_subroutine_node * self = (jive_subroutine_node *) self_;
	jive_subroutine_deprecated * subroutine = self->operation().subroutine();
	if (subroutine) {
		JIVE_DEBUG_ASSERT(subroutine->subroutine_node == self);
		subroutine->subroutine_node = 0;
		jive_subroutine_destroy(subroutine);
	}
	jive_node_fini_(self);
}

jive_node *
jive_subroutine_node_create(
	jive_region * subroutine_region,
	jive_subroutine_deprecated * subroutine)
{
	jive_region * region = subroutine_region->parent;
	
	JIVE_DEBUG_ASSERT(region);
	
	JIVE_DEBUG_ASSERT(subroutine_region->top && subroutine_region->bottom);
	
	JIVE_DEBUG_ASSERT(jive_node_isinstance(subroutine_region->top, &JIVE_SUBROUTINE_ENTER_NODE));
	JIVE_DEBUG_ASSERT(jive_node_isinstance(subroutine_region->bottom, &JIVE_SUBROUTINE_LEAVE_NODE));
	
	jive_subroutine_enter_node * enter = (jive_subroutine_enter_node *) subroutine_region->top;
	jive_subroutine_leave_node * leave = (jive_subroutine_leave_node *) subroutine_region->bottom;
	
	jive_subroutine_node * node = jive::subroutine_op(subroutine).create_node(
		region, leave->noutputs, &leave->outputs[0]);

	subroutine->subroutine_node = node;
	subroutine->enter = enter;
	subroutine->leave = leave;
	
	return node;
}
