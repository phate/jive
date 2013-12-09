/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/subroutine.h>
#include <jive/arch/subroutine-private.h>

#include <string.h>

#include <jive/common.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/resource.h>

static void
jive_subroutine_enter_node_fini_(jive_node * self_);

static const jive_node_attrs *
jive_subroutine_enter_node_get_attrs_(const jive_node * self_);

static bool
jive_subroutine_enter_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_);

static jive_node *
jive_subroutine_enter_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_SUBROUTINE_ENTER_NODE = {
	.parent = &JIVE_NODE,
	.name = "SUBROUTINE_ENTER",
	.fini = jive_subroutine_enter_node_fini_, /* override */
	.get_default_normal_form = jive_node_get_default_normal_form_, /* inherit */
	.get_label = jive_node_get_label_, /* inherit */
	.get_attrs = jive_subroutine_enter_node_get_attrs_, /* override */
	.match_attrs = jive_subroutine_enter_node_match_attrs_, /* override */
	.check_operands = jive_node_check_operands_, /* inherit */
	.create = jive_subroutine_enter_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

static void
jive_subroutine_enter_node_fini_(jive_node * self_)
{
	jive_subroutine_enter_node * self = (jive_subroutine_enter_node *) self_;
	jive_subroutine * subroutine = self->attrs.subroutine;
	if (subroutine) {
		JIVE_DEBUG_ASSERT(subroutine->enter == self);
		subroutine->enter = 0;
	}
	jive_node_fini_(&self->base);
}

static const jive_node_attrs *
jive_subroutine_enter_node_get_attrs_(const jive_node * self_)
{
	const jive_subroutine_enter_node * self = (const jive_subroutine_enter_node *) self_;
	return &self->attrs.base;
}

static bool
jive_subroutine_enter_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_)
{
	const jive_subroutine_enter_node * self = (const jive_subroutine_enter_node *) self_;
	const jive_subroutine_node_attrs * attrs = (const jive_subroutine_node_attrs *) attrs_;
	return self->attrs.subroutine == attrs->subroutine;
}

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
	jive_subroutine_enter_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	
	node->base.class_ = &JIVE_SUBROUTINE_ENTER_NODE;
	JIVE_DECLARE_CONTROL_TYPE(ctl);
	jive_node_init_(&node->base, region,
		0, NULL, NULL,
		1, &ctl);
	((jive_control_output *) node->base.outputs[0])->active = false;
	node->attrs.subroutine = 0;
	region->top = &node->base;
	
	return &node->base;
}

static void
jive_subroutine_leave_node_fini_(jive_node * self_);

static const jive_node_attrs *
jive_subroutine_leave_node_get_attrs_(const jive_node * self_);

static bool
jive_subroutine_leave_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_);

static jive_node *
jive_subroutine_leave_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_SUBROUTINE_LEAVE_NODE = {
	.parent = &JIVE_NODE,
	.name = "SUBROUTINE_LEAVE",
	.fini = jive_subroutine_leave_node_fini_, /* override */
	.get_default_normal_form = jive_node_get_default_normal_form_, /* inherit */
	.get_label = jive_node_get_label_, /* inherit */
	.get_attrs = jive_subroutine_leave_node_get_attrs_, /* override */
	.match_attrs = jive_subroutine_leave_node_match_attrs_, /* override */
	.check_operands = jive_node_check_operands_, /* inherit */
	.create = jive_subroutine_leave_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

static void
jive_subroutine_leave_node_fini_(jive_node * self_)
{
	jive_subroutine_leave_node * self = (jive_subroutine_leave_node *) self_;
	jive_subroutine * subroutine = self->attrs.subroutine;
	if (subroutine) {
		JIVE_DEBUG_ASSERT(subroutine->leave == self);
		subroutine->leave = 0;
	}
	jive_node_fini_(&self->base);
}

static const jive_node_attrs *
jive_subroutine_leave_node_get_attrs_(const jive_node * self_)
{
	const jive_subroutine_leave_node * self = (const jive_subroutine_leave_node *) self_;
	return &self->attrs.base;
}

static bool
jive_subroutine_leave_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_)
{
	const jive_subroutine_leave_node * self = (const jive_subroutine_leave_node *) self_;
	const jive_subroutine_node_attrs * attrs = (const jive_subroutine_node_attrs *) attrs_;
	return self->attrs.subroutine == attrs->subroutine;
}

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
	jive_subroutine_leave_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	
	node->base.class_ = &JIVE_SUBROUTINE_LEAVE_NODE;
	JIVE_DECLARE_CONTROL_TYPE(ctl);
	JIVE_DECLARE_ANCHOR_TYPE(anchor);
	jive_node_init_(&node->base, region,
		1, &ctl, &control_transfer,
		1, &anchor);
	node->attrs.subroutine = 0;
	region->bottom = &node->base;
	
	return &node->base;
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
	.parent = &JIVE_NODE,
	.name = "SUBROUTINE",
	.fini = jive_subroutine_node_fini_, /* override */
	.get_default_normal_form = jive_node_get_default_normal_form_, /* inherit */
	.get_label = jive_node_get_label_, /* inherit */
	.get_attrs = jive_subroutine_node_get_attrs_, /* override */
	.match_attrs = jive_subroutine_node_match_attrs_, /* override */
	.check_operands = jive_node_check_operands_, /* inherit */
	.create = jive_subroutine_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

static void
jive_subroutine_node_fini_(jive_node * self_)
{
	jive_subroutine_node * self = (jive_subroutine_node *) self_;
	jive_subroutine * subroutine = self->attrs.subroutine;
	if (subroutine) {
		JIVE_DEBUG_ASSERT(subroutine->subroutine_node == self);
		subroutine->subroutine_node = 0;
		jive_subroutine_destroy(subroutine);
	}
	jive_node_fini_(&self->base);
}
static const jive_node_attrs *
jive_subroutine_node_get_attrs_(const jive_node * self_)
{
	const jive_subroutine_node * self = (const jive_subroutine_node *) self_;
	return &self->attrs.base;
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
	
	jive_subroutine * subroutine = attrs->subroutine;
	subroutine = subroutine->class_->copy(subroutine, subroutine_region->top, subroutine_region->bottom);
	
	return jive_subroutine_node_create(operands[0]->node->region, subroutine);
}

jive_node *
jive_subroutine_node_create(jive_region * subroutine_region, jive_subroutine * subroutine)
{
	jive_region * region = subroutine_region->parent;
	
	JIVE_DEBUG_ASSERT(region);
	
	JIVE_DEBUG_ASSERT(subroutine_region->top && subroutine_region->bottom);
	
	JIVE_DEBUG_ASSERT(jive_node_isinstance(subroutine_region->top, &JIVE_SUBROUTINE_ENTER_NODE));
	JIVE_DEBUG_ASSERT(jive_node_isinstance(subroutine_region->bottom, &JIVE_SUBROUTINE_LEAVE_NODE));
	
	jive_subroutine_enter_node * enter = (jive_subroutine_enter_node *) subroutine_region->top;
	jive_subroutine_leave_node * leave = (jive_subroutine_leave_node *) subroutine_region->bottom;
	
	JIVE_DEBUG_ASSERT(!enter->attrs.subroutine);
	JIVE_DEBUG_ASSERT(!leave->attrs.subroutine);
	
	jive_subroutine_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	
	node->base.class_ = &JIVE_SUBROUTINE_NODE;
	JIVE_DECLARE_STATE_TYPE(objstate_type);
	JIVE_DECLARE_ANCHOR_TYPE(anchor);
	jive_node_init_(&node->base, region,
		1, &anchor, &subroutine_region->bottom->outputs[0],
		1, &objstate_type);
	node->attrs.subroutine = subroutine;
	enter->attrs.subroutine = subroutine;
	leave->attrs.subroutine = subroutine;
	subroutine->subroutine_node = node;
	subroutine->enter = enter;
	subroutine->leave = leave;
	
	return &node->base;
}

void
jive_subroutine_destroy(jive_subroutine * self)
{
	if (self->enter)
		self->enter->attrs.subroutine = 0;
	if (self->leave)
		self->leave->attrs.subroutine = 0;
	if (self->subroutine_node)
		self->subroutine_node->attrs.subroutine = 0;
	self->class_->fini(self);
	jive_context_free(self->context, self);
}

void
jive_subroutine_create_region_and_nodes(jive_subroutine * subroutine, jive_region * parent_region)
{
	jive_region * subroutine_region = jive_region_create_subregion(parent_region);
	subroutine_region->attrs.section = jive_region_section_code;
	jive_subroutine_leave_node_create(subroutine_region, jive_subroutine_enter_node_create(subroutine_region)->outputs[0]);
	jive_subroutine_node_create(subroutine_region, subroutine);
	subroutine->region = subroutine_region;
}

jive_subroutine_passthrough
jive_subroutine_create_passthrough(jive_subroutine * subroutine, const jive_resource_class * cls, const char * name)
{
	jive_subroutine_passthrough passthrough;
	passthrough.gate = jive_resource_class_create_gate(cls, subroutine->subroutine_node->base.region->graph, name);
	passthrough.output = jive_node_gate_output(&subroutine->enter->base, passthrough.gate);
	passthrough.input = jive_node_gate_input(&subroutine->leave->base, passthrough.gate, passthrough.output);
	return passthrough;
}

jive_gate *
jive_subroutine_match_gate(jive_gate * gate, jive_node * old_node, jive_node * new_node)
{
	size_t n;
	
	JIVE_DEBUG_ASSERT(new_node->ninputs >= old_node->ninputs);
	for (n = 0; n < old_node->ninputs; n++) {
		if (old_node->inputs[n]->gate == gate)
			return new_node->inputs[n]->gate;
	}
	
	JIVE_DEBUG_ASSERT(new_node->noutputs >= old_node->noutputs);
	for (n = 0; n < old_node->noutputs; n++) {
		if (old_node->outputs[n]->gate == gate)
			return new_node->outputs[n]->gate;
	}
	
	return NULL;
}

void
jive_subroutine_init_(jive_subroutine * self, const jive_subroutine_class * cls,
	jive_context * context, const struct jive_instructionset * instructionset,
	size_t nparameters, const jive_argument_type parameter_types[],
	size_t nreturns, const jive_argument_type return_types[],
	size_t npassthroughs)
{
	self->class_ = cls;
	
	self->context = context;
	
	self->enter = NULL;
	self->leave = NULL;
	self->subroutine_node = NULL;
	
	self->region = NULL;
	
	self->frame.lower_bound = 0;
	self->frame.upper_bound = 0;
	self->frame.frame_pointer_offset = 0;
	self->frame.stack_pointer_offset = 0;
	self->frame.call_area_size = 0;
	self->instructionset = instructionset;
	
	size_t n;
	
	self->nparameters = nparameters;
	self->parameters = jive_context_malloc(context, sizeof(self->parameters[0]) * nparameters);
	self->parameter_types = jive_context_malloc(context,
		sizeof(self->parameter_types[0]) * nparameters);
	for (n = 0; n < nparameters; n++) {
		self->parameters[n] = NULL;
		self->parameter_types[n] = parameter_types[n];
	}
	
	self->nreturns = nreturns;
	self->returns = jive_context_malloc(context, sizeof(self->returns[0]) * nreturns);
	self->return_types = jive_context_malloc(context, sizeof(self->returns[0]) * nreturns);
	for (n = 0; n < nreturns; n++) {
		self->returns[n] = NULL;
		self->return_types[n] = return_types[n];
	}
	
	self->npassthroughs = npassthroughs;
	self->passthroughs = jive_context_malloc(context, sizeof(self->passthroughs[0]) * npassthroughs);
	for (n = 0; n < npassthroughs; n++) {
		self->passthroughs[n].gate = NULL;
		self->passthroughs[n].input = NULL;
		self->passthroughs[n].output = NULL;
	}
}

void
jive_subroutine_fini_(jive_subroutine * self)
{
	jive_context * context = self->context;
	jive_context_free(context, self->passthroughs);
	jive_context_free(context, self->parameters);
	jive_context_free(context, self->parameter_types);
	jive_context_free(context, self->returns);
	jive_context_free(context, self->return_types);
}
