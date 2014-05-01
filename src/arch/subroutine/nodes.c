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

jive_subroutine_deprecated *
jive_subroutine_copy(const jive_subroutine_deprecated * self,
	jive_node * new_enter_node, jive_node * new_leave_node);

static jive_node *
jive_subroutine_enter_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_SUBROUTINE_ENTER_NODE = {
	parent : &JIVE_NODE,
	name : "SUBROUTINE_ENTER",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : jive_node_get_label_, /* inherit */
	get_attrs : jive_node_get_attrs_, /* inherit */
	match_attrs : jive_node_match_attrs_, /* inherit */
	check_operands : jive_node_check_operands_, /* inherit */
	create : jive_subroutine_enter_node_create_, /* override */
};

static jive_node *
jive_subroutine_enter_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 0);
	return jive_subroutine_enter_node_create(region);
}

jive_node *
jive_subroutine_enter_node_create(jive_region * region)
{
	JIVE_DEBUG_ASSERT(region->top == NULL && region->bottom == NULL);
	jive_subroutine_enter_node * node = new jive_subroutine_enter_node;
	
	node->class_ = &JIVE_SUBROUTINE_ENTER_NODE;
	JIVE_DECLARE_CONTROL_TYPE(ctl);
	jive_node_init_(node, region,
		0, NULL, NULL,
		1, &ctl);
	((jive_control_output *) node->outputs[0])->set_active(false);
	region->top = node;
	
	return node;
}

static jive_node *
jive_subroutine_leave_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_SUBROUTINE_LEAVE_NODE = {
	parent : &JIVE_NODE,
	name : "SUBROUTINE_LEAVE",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : jive_node_get_label_, /* inherit */
	get_attrs : jive_node_get_attrs_, /* inherit */
	match_attrs : jive_node_match_attrs_, /* inherit */
	check_operands : jive_node_check_operands_, /* inherit */
	create : jive_subroutine_leave_node_create_, /* override */
};

static jive_node *
jive_subroutine_leave_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);
	return jive_subroutine_leave_node_create(region, operands[0]);
}

jive_node *
jive_subroutine_leave_node_create(jive_region * region, jive_output * control_transfer)
{
	JIVE_DEBUG_ASSERT(region->bottom == NULL);
	jive_subroutine_leave_node * node = new jive_subroutine_leave_node;
	
	node->class_ = &JIVE_SUBROUTINE_LEAVE_NODE;
	JIVE_DECLARE_CONTROL_TYPE(ctl);
	jive_anchor_type anchor;
	const jive_type * ancptr = &anchor;
	jive_node_init_(node, region,
		1, &ctl, &control_transfer,
		1, &ancptr);
	region->bottom = node;
	
	return node;
}

static void
jive_subroutine_node_fini_(jive_node * self_);

static const jive_node_attrs *
jive_subroutine_node_get_attrs_(const jive_node * self_);

static bool
jive_subroutine_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_);

static jive_node *
jive_subroutine_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_SUBROUTINE_NODE = {
	parent : &JIVE_NODE,
	name : "SUBROUTINE",
	fini : jive_subroutine_node_fini_, /* override */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : jive_node_get_label_, /* inherit */
	get_attrs : jive_subroutine_node_get_attrs_, /* override */
	match_attrs : jive_subroutine_node_match_attrs_, /* override */
	check_operands : jive_node_check_operands_, /* inherit */
	create : jive_subroutine_node_create_, /* override */
};

static void
jive_subroutine_node_fini_(jive_node * self_)
{
	jive_subroutine_node * self = (jive_subroutine_node *) self_;
	jive_subroutine_deprecated * subroutine = self->attrs.subroutine;
	if (subroutine) {
		JIVE_DEBUG_ASSERT(subroutine->subroutine_node == self);
		subroutine->subroutine_node = 0;
		jive_subroutine_destroy(subroutine);
	}
	jive_node_fini_(self);
}
static const jive_node_attrs *
jive_subroutine_node_get_attrs_(const jive_node * self_)
{
	const jive_subroutine_node * self = (const jive_subroutine_node *) self_;
	return &self->attrs;
}

static bool
jive_subroutine_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_)
{
	const jive_subroutine_node * self = (const jive_subroutine_node *) self_;
	const jive_subroutine_node_attrs * attrs = (const jive_subroutine_node_attrs *) attrs_;
	return self->attrs.subroutine == attrs->subroutine;
}

static jive_node *
jive_subroutine_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);
	JIVE_DEBUG_ASSERT(operands[0]->node->region->parent == region);
	jive_region * subroutine_region = operands[0]->node->region;
	
	const jive_subroutine_node_attrs * attrs = (const jive_subroutine_node_attrs *) attrs_;
	
	jive_subroutine_deprecated * subroutine = attrs->subroutine;
	subroutine = jive_subroutine_copy(subroutine, subroutine_region->top, subroutine_region->bottom);
	
	return jive_subroutine_node_create(operands[0]->node->region, subroutine);
}

jive_node *
jive_subroutine_node_create(jive_region * subroutine_region, jive_subroutine_deprecated * subroutine)
{
	jive_region * region = subroutine_region->parent;
	
	JIVE_DEBUG_ASSERT(region);
	
	JIVE_DEBUG_ASSERT(subroutine_region->top && subroutine_region->bottom);
	
	JIVE_DEBUG_ASSERT(jive_node_isinstance(subroutine_region->top, &JIVE_SUBROUTINE_ENTER_NODE));
	JIVE_DEBUG_ASSERT(jive_node_isinstance(subroutine_region->bottom, &JIVE_SUBROUTINE_LEAVE_NODE));
	
	jive_subroutine_enter_node * enter = (jive_subroutine_enter_node *) subroutine_region->top;
	jive_subroutine_leave_node * leave = (jive_subroutine_leave_node *) subroutine_region->bottom;
	
	jive_subroutine_node * node = new jive_subroutine_node;
	
	node->class_ = &JIVE_SUBROUTINE_NODE;
	/*
		FIXME: a subroutine node should not have a memory type as output, put it is the only type
						we have right now
	*/
	jive_memory_type objstate_type;
	const jive_type * typeptr = &objstate_type;
	jive_anchor_type anchor;
	const jive_type * ancptr = &anchor;
	jive_node_init_(node, region,
		1, &ancptr, &subroutine_region->bottom->outputs[0],
		1, &typeptr);
	node->attrs.subroutine = subroutine;
	subroutine->subroutine_node = node;
	subroutine->enter = enter;
	subroutine->leave = leave;
	
	return node;
}
