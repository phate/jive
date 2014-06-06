/*
 * Copyright 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/theta.h>

#include <string.h>
#include <stdio.h>

#include <jive/common.h>
#include <jive/context.h>
#include <jive/vsdg/anchor-private.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

static jive_node *
jive_theta_head_node_create_(jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[])
{
	jive_node * self = jive::create_operation_node(jive_op_theta_head());
	jive::ctl::type control;
	const jive::base::type * control_ptr = &control;
	self->class_ = &JIVE_THETA_HEAD_NODE;
	jive_node_init_(self, region,
		0, NULL, NULL,
		1, &control_ptr);
	
	region->top = self;
	return self;
}

static jive_node *
jive_theta_tail_node_create_(jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);
	jive_node * self = jive::create_operation_node(jive_op_theta_tail());
	jive::achr::type anchor;
	const jive::base::type * ancptr = &anchor;
	jive::ctl::type control;
	const jive::base::type * control_ptr = &control;
	self->class_ = &JIVE_THETA_TAIL_NODE;
	jive_node_init_(self, region,
		1, &control_ptr, operands,
		1, &ancptr);
	
	region->bottom = self;
	return self;
}

static jive_node *
jive_theta_node_create_(jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);
	jive_node * self = jive::create_operation_node(jive_op_theta());
	jive::achr::type anchor;
	const jive::base::type * ancptr = &anchor;
	self->class_ = &JIVE_THETA_NODE;
	jive_node_init_(self, region,
		1, &ancptr, operands,
		0, NULL);
	
	return self;
}

const jive_node_class JIVE_THETA_HEAD_NODE = {
	parent : &JIVE_NODE,
	name : "THETA_HEAD",
	fini : jive_node_fini_,  /* inherit */
	get_default_normal_form : jive_node_get_default_normal_form_,  /* inherit */
	get_label : jive_node_get_label_,  /* inherit */
	match_attrs : jive_node_match_attrs_,  /* inherit */
	check_operands : jive_node_check_operands_, /* inherrit */
	create : jive_theta_head_node_create_,  /* override */
};

const jive_node_class JIVE_THETA_TAIL_NODE = {
	parent : &JIVE_NODE,
	name : "THETA_TAIL",
	fini : jive_node_fini_,  /* inherit */
	get_default_normal_form : jive_node_get_default_normal_form_,  /* inherit */
	get_label : jive_node_get_label_,  /* inherit */
	match_attrs : jive_node_match_attrs_,  /* inherit */
	check_operands : jive_node_check_operands_, /* inherrit */
	create : jive_theta_tail_node_create_,  /* override */
};

const jive_node_class JIVE_THETA_NODE = {
	parent : &JIVE_ANCHOR_NODE,
	name : "THETA",
	fini : jive_node_fini_,  /* inherit */
	get_default_normal_form : jive_anchor_node_get_default_normal_form_,  /* inherit */
	get_label : jive_node_get_label_,  /* inherit */
	match_attrs : jive_node_match_attrs_,  /* inherit */
	check_operands : jive_node_check_operands_, /* inherrit */
	create : jive_theta_node_create_,  /* override */
};

static jive_node *
jive_theta_head_node_create(jive_region * region)
{
	jive_node * self = jive::create_operation_node(jive_op_theta_head());
	jive::ctl::type control;
	const jive::base::type * control_ptr = &control;
	self->class_ = &JIVE_THETA_HEAD_NODE;
	jive_node_init_(self, region,
		0, NULL, NULL,
		1, &control_ptr);
	
	region->top = self;
	return self;
}

static jive_node *
jive_theta_tail_node_create(jive_region * region, jive_output * predicate)
{
	jive_node * self = jive::create_operation_node(jive_op_theta_tail());
	jive::achr::type anchor;
	const jive::base::type * ancptr = &anchor;
	jive::ctl::type control;
	const jive::base::type * control_ptr = &control;
	self->class_ = &JIVE_THETA_TAIL_NODE;
	jive_node_init_(self, region,
		1, &control_ptr, &predicate,
		1, &ancptr);
	
	region->bottom = self;
	return self;
}

static jive_node *
jive_theta_node_create(jive_region * region,
	jive_output * loop_body)
{
	jive_node * self = jive::create_operation_node(jive_op_theta());
	jive::achr::type anchor;
	const jive::base::type * ancptr = &anchor;
	self->class_ = &JIVE_THETA_NODE;
	jive_node_init_(self, region,
		1, &ancptr, &loop_body,
		0, NULL);
	
	return self;
}

jive_node *
jive_theta_create(
	jive_region * region,
	size_t nvalues, const jive::base::type * const * types, jive_output * const * values)
{
	jive_region * loop_region = jive_region_create_subregion(region);
	jive_node * head = jive_theta_head_node_create(loop_region);
	jive_node * tail = jive_theta_tail_node_create(loop_region, head->outputs[0]);
	jive_node * theta = jive_theta_node_create(region, tail->outputs[0]);
	
	size_t n;
	for (n = 0; n < nvalues; n++) {
		char name[80];
		snprintf(name, sizeof(name), "theta_%p_%zd", theta, n);
		jive_gate * gate = types[n]->create_gate(region->graph, name);
		jive_node_gate_input(head, gate, values[n]);
		jive_output * inner = jive_node_gate_output(head, gate);
		jive_node_gate_input(tail, gate, inner);
		jive_node_gate_output(theta, gate);
	}
	return theta;
}

typedef struct jive_theta_build_state jive_theta_build_state;
struct jive_theta_build_state {
	size_t nloopvars;
	jive_theta_loopvar * loopvars;
	jive_floating_region floating;
};

jive_theta
jive_theta_begin(jive_graph * graph)
{
	jive_theta self;
	jive_theta_build_state * state;
	state = jive_context_malloc(graph->context, sizeof(*state));
	state->floating = jive_floating_region_create(graph);
	self.region = state->floating.region;
	state->nloopvars = 0;
	state->loopvars = 0;
	
	state->floating.region->attrs.is_looped = true;
	jive_theta_head_node_create(self.region);
	
	self.internal_state = state;
	
	return self;
}

jive_theta_loopvar
jive_theta_loopvar_enter(jive_theta self, struct jive_output * pre_value)
{
	jive_theta_build_state * state = self.internal_state;
	jive_node * head = self.region->top;
	jive_graph * graph = head->region->graph;
	jive_context * context = graph->context;
	
	size_t index = state->nloopvars;
	
	const jive::base::type * type = &pre_value->type();
	++ state->nloopvars;
	state->loopvars = jive_context_realloc(context,
		state->loopvars, sizeof(state->loopvars[0]) * state->nloopvars);
	
	char gate_name[80];
	snprintf(gate_name, sizeof(gate_name), "loopvar_%p_%zd", head, index);
	state->loopvars[index].gate = type->create_gate(graph, gate_name);
	jive_node_gate_input(head, state->loopvars[index].gate, pre_value);
	state->loopvars[index].value = jive_node_gate_output(head,
		state->loopvars[index].gate);
	
	return state->loopvars[index];
}

void
jive_theta_loopvar_leave(jive_theta self, jive_gate * var,
	struct jive_output * post_value)
{
	jive_theta_build_state * state = self.internal_state;
	size_t n;
	for (n = 0; n < state->nloopvars; ++n) {
		if (state->loopvars[n].gate != var)
			continue;
		state->loopvars[n].value = post_value;
		return;
	}
	
	jive_context_fatal_error(self.region->graph->context,
		"Lookup of loop-variant variable failed");
}

jive_node *
jive_theta_end(jive_theta self, jive_output * predicate,
	size_t npost_values, jive_theta_loopvar * post_values)
{
	jive_theta_build_state * state = self.internal_state;
	jive_node * head = self.region->top; 
	jive_graph * graph = head->region->graph;
	jive_context * context = graph->context;
	
	size_t n;
	
	jive_node * tail = jive_theta_tail_node_create(self.region,
		predicate);
	for (n = 0; n < state->nloopvars; ++n)
		jive_node_gate_input(tail, state->loopvars[n].gate, state->loopvars[n].value);
	
	jive_floating_region_settle(state->floating);
	
	jive_node * anchor = jive_theta_node_create(self.region->parent, tail->outputs[0]);
	for (n = 0; n < state->nloopvars; ++n)
		state->loopvars[n].value = jive_node_gate_output(anchor, state->loopvars[n].gate);
	
	for (n = 0; n < npost_values; ++n) {
		size_t k;
		for (k = 0; k < state->nloopvars; ++k) {
			if (state->loopvars[k].gate == post_values[n].gate) {
				post_values[n].value = state->loopvars[k].value;
				break;
			}
		}
	}
	
	jive_context_free(context, state->loopvars);
	jive_context_free(context, state);
	
	return anchor;
}

