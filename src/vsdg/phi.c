/*
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/phi.h>

#include <stdio.h>

#include <jive/vsdg/anchor-private.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/valuetype.h>
#include <jive/vsdg/substitution.h>

/* phi node normal form */

static bool
jive_phi_node_normal_form_normalize_node_(const jive_node_normal_form * self_, jive_node * node)
{
	const jive_phi_node_normal_form * self = (const jive_phi_node_normal_form *) self_;
	if (!self->base.enable_mutable)
		return true;

	const jive_node_attrs * attrs = jive_node_get_attrs(node);

	if (self->base.enable_cse) {
		jive_output * tmp = node->inputs[0]->origin();
		jive_node * new_node = jive_node_cse(node->region, self->base.node_class, attrs, 1, &tmp);
		JIVE_DEBUG_ASSERT(new_node);
		if (new_node != node) {
			jive_output_replace(node->outputs[0], new_node->outputs[0]);
			/* FIXME: not sure whether "destroy" is really appropriate */
			jive_node_destroy(node);
			return false;
		}
	}

	return true;
}

static bool
jive_phi_node_normal_form_operands_are_normalized_(const jive_node_normal_form * self_,
	size_t noperands, jive_output * const operands[], const jive_node_attrs * attrs)
{
	const jive_phi_node_normal_form * self = (const jive_phi_node_normal_form *) self_;
	if (!self->base.enable_mutable)
		return true;

	JIVE_DEBUG_ASSERT(noperands == 1);

	jive_region * region = operands[0]->node()->region;
	const jive_node_class * cls = self->base.node_class;

	if (self->base.enable_cse && jive_node_cse(region, cls, attrs, noperands, operands))
		return false;

	return true;
}

static void
jive_phi_node_normalized_create_(const jive_phi_node_normal_form * self,
	struct jive_region * phi_region, jive_output  * results[])
{
	const jive_node_class * cls = self->base.node_class;

	JIVE_DEBUG_ASSERT(jive_region_get_bottom_node(phi_region)->noutputs == 1);
	jive_node * leave = jive_region_get_bottom_node(phi_region); 
	jive_output * operand = leave->outputs[0];

	if (!self->base.enable_mutable) {
		size_t n;
		jive_node * node = cls->create(phi_region->parent, NULL, 1, &operand);
		for (n = 0; n < node->noutputs; n++)
			results[n] = node->outputs[n];
		return;
	}

	if (self->base.enable_cse) {
		size_t n;
		jive_node * node = jive_node_cse(phi_region, cls, NULL, 1, &operand);
		if (node) {
			for (n = 0; n < node->noutputs; n++)
				results[n] = node->outputs[n];
			return;
		}
	}

	size_t n;
	jive_node * node = cls->create(phi_region->parent, NULL, 1, &operand);
	for (n = 0; n < node->noutputs; n++)
		results[n] = node->outputs[n];
	return;
}

const jive_phi_node_normal_form_class JIVE_PHI_NODE_NORMAL_FORM_ = {
	base : { /* jive_anchor_node_normal_form_class */
		base : { /* jive_node_normal_form_class */
			parent : &JIVE_ANCHOR_NODE_NORMAL_FORM,
			fini : jive_node_normal_form_fini_, /* inherit */
			normalize_node : jive_phi_node_normal_form_normalize_node_, /* override */
			operands_are_normalized : jive_phi_node_normal_form_operands_are_normalized_, /* override */
			normalized_create : NULL,
			set_mutable : jive_node_normal_form_set_mutable_, /* inherit */
			set_cse : jive_node_normal_form_set_cse_ /* inherit */
		},
		set_reducible : jive_anchor_node_normal_form_set_reducible_ /* inherit */
	},
	normalized_create : jive_phi_node_normalized_create_
};

/* phi enter node */

static jive_node *
jive_phi_enter_node_create(jive_region * region)
{
	JIVE_DEBUG_ASSERT(region->top == NULL && region->bottom == NULL);
	jive_node * node = jive::create_operation_node(jive_op_phi_enter());

	node->class_ = &JIVE_PHI_ENTER_NODE;
	jive::ctl::type ctltype;
	const jive_type * ctltype_ptr = &ctltype;
	jive_node_init_(node, region,
		0, NULL, NULL,
		1, &ctltype_ptr);
	static_cast<jive::ctl::output*>(node->outputs[0])->set_active(false);
	region->top = node;
	
	return node;
}

static jive_node *
jive_phi_enter_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 0);

	return jive_phi_enter_node_create(region);
}

const jive_node_class JIVE_PHI_ENTER_NODE = {
	parent : &JIVE_NODE,
	name : "PHI_ENTER",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : jive_node_get_label_, /* inherit */
	match_attrs : jive_node_match_attrs_, /* inherit */
	check_operands : jive_node_check_operands_, /* inherrit */
	create : jive_phi_enter_node_create_, /* override */
};

/* phi leave node */

static jive_node *
jive_phi_leave_node_create(jive_region * region)
{
	JIVE_DEBUG_ASSERT(region->top != NULL);
	JIVE_DEBUG_ASSERT(region->bottom == NULL);
	jive_node * node = jive::create_operation_node(jive_op_phi_leave());

	node->class_ = &JIVE_PHI_LEAVE_NODE;
	jive_anchor_type anctype;
	const jive_type * ancptr = &anctype;
	jive::ctl::type ctltype;
	const jive_type *ctltype_ptr = &ctltype;
	jive_node_init_(node, region,
		1, &ctltype_ptr, &region->top->outputs[0],
		1, &ancptr);
	region->bottom = node;

	return node;
}

static jive_node *
jive_phi_leave_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 0);

	return jive_phi_leave_node_create(region);
}

const jive_node_class JIVE_PHI_LEAVE_NODE = {
	parent : &JIVE_NODE,
	name : "PHI_LEAVE",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : jive_node_get_label_, /* inherit */
	match_attrs : jive_node_match_attrs_, /* inherit */
	check_operands : jive_node_check_operands_, /* inherrit */
	create : jive_phi_leave_node_create_, /* override */
};

/* phi node */

static jive_node_normal_form *
jive_phi_node_get_default_normal_form_(const jive_node_class * cls,
	jive_node_normal_form * parent_, struct jive_graph * graph);

static jive_node *
jive_phi_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_PHI_NODE = {
	parent : &JIVE_ANCHOR_NODE,
	name : "PHI",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_phi_node_get_default_normal_form_, /* override */
	get_label : jive_node_get_label_, /* inherit */
	match_attrs : jive_node_match_attrs_, /* inherit */
	check_operands : jive_node_check_operands_, /* inherrit */
	create : jive_phi_node_create_, /* override */
};


static jive_node_normal_form *
jive_phi_node_get_default_normal_form_(const jive_node_class * cls,
	jive_node_normal_form * parent_, struct jive_graph * graph)
{
	jive_context * context = graph->context;
	jive_phi_node_normal_form * nf = jive_context_malloc(context, sizeof(*nf));
	nf->base.class_ = &JIVE_PHI_NODE_NORMAL_FORM;

	jive_anchor_node_normal_form_init_(nf, cls, parent_, graph);

	return &nf->base;
}

static jive_node *
jive_phi_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);
	jive_node * self = jive::create_operation_node(jive_op_phi());
	jive_anchor_type anchor;
	const jive_type *anchorptr = &anchor;
	self->class_ = &JIVE_PHI_NODE;
	jive_node_init_(self, region,
		1, &anchorptr, operands,
		0, NULL);
	
	return self;
}

static jive_node *
jive_phi_node_create(jive_region * phi_region,
	jive_output * phi_body)
{
	jive_phi_node * self = new jive_phi_node(jive_op_phi());
	jive_anchor_type anchor;
	const jive_type * ancptr = &anchor;
	self->class_ = &JIVE_PHI_NODE;
	jive_node_init_(self, phi_region,
		1, &ancptr, &phi_body,
		0, NULL);
	
	return self;
}

/* phi instantiation */

typedef struct jive_phi_build_state jive_phi_build_state;
struct jive_phi_build_state {
	size_t nfixvars;
	jive_phi_fixvar * fixvars;
	jive_floating_region floating;
};

jive_phi
jive_phi_begin(jive_graph * graph)
{
	jive_phi self;
	jive_phi_build_state * state;
	state = jive_context_malloc(graph->context, sizeof(*state));
	state->floating = jive_floating_region_create(graph);
	self.region = state->floating.region;
	state->nfixvars = 0;
	state->fixvars = 0;
	
	jive_phi_enter_node_create(self.region);
	
	self.internal_state = state;
	
	return self;
}

jive_phi_fixvar
jive_phi_fixvar_enter(jive_phi self, const struct jive_type * type)
{
	jive_phi_build_state * state = self.internal_state;
	jive_node * enter = self.region->top;
	jive_graph * graph = enter->region->graph;
	jive_context * context = graph->context;
	
	size_t index = state->nfixvars;
	
	++ state->nfixvars;
	state->fixvars = jive_context_realloc(context,
		state->fixvars, sizeof(state->fixvars[0]) * state->nfixvars);
	
	char gate_name[80];
	snprintf(gate_name, sizeof(gate_name), "fix_%p_%zd", enter, index);
	state->fixvars[index].gate = type->create_gate(graph, gate_name);
	state->fixvars[index].value = jive_node_gate_output(enter, state->fixvars[index].gate);
	
	return state->fixvars[index];
}

void
jive_phi_fixvar_leave(jive_phi self, jive_gate * var,
	struct jive_output * fix_value)
{
	jive_phi_build_state * state = self.internal_state;
	size_t n;
	for (n = 0; n < state->nfixvars; ++n) {
		if (state->fixvars[n].gate != var)
			continue;
		state->fixvars[n].value = fix_value;
		return;
	}
	
	jive_context_fatal_error(self.region->graph->context,
		"Lookup of fix point variable failed");
}

jive_node *
jive_phi_end(jive_phi self,
	size_t npost_values, jive_phi_fixvar * fix_values)
{
	jive_phi_build_state * state = self.internal_state;
	jive_node * enter = self.region->top;
	jive_graph * graph = enter->region->graph;
	jive_context * context = graph->context;
	
	size_t n;
	
	jive_node * leave = jive_phi_leave_node_create(enter->region);
	for (n = 0; n < state->nfixvars; ++n)
		jive_node_gate_input(leave, state->fixvars[n].gate, state->fixvars[n].value);
	
	jive_floating_region_settle(state->floating);
	
	jive_node * anchor = jive_phi_node_create(self.region->parent, leave->outputs[0]);
	for (n = 0; n < state->nfixvars; ++n)
		state->fixvars[n].value = jive_node_gate_output(anchor, state->fixvars[n].gate);
	
	for (n = 0; n < npost_values; ++n) {
		size_t k;
		for (k = 0; k < state->nfixvars; ++k) {
			if (state->fixvars[k].gate == fix_values[n].gate) {
				fix_values[n].value = state->fixvars[k].value;
				break;
			}
		}
		if (k == state->nfixvars)
			jive_context_fatal_error(self.region->graph->context,
				"Lookup of fix point variable failed");
	}
	
	jive_context_free(context, state->fixvars);
	jive_context_free(context, state);
	
	return anchor;
}

struct jive_phi_extension *
jive_phi_begin_extension(jive_phi_node * phi_node, size_t nfixvars,
	const jive_type * fixvar_types[])
{
	jive_graph * graph = phi_node->region->graph;
	jive_context * context = graph->context;
	jive_node * enter = jive_phi_node_get_enter_node(phi_node);

	jive_phi_extension * phi_ext = jive_context_malloc(context, sizeof(*phi_ext));
	phi_ext->fixvars = jive_context_malloc(context, sizeof(*phi_ext->fixvars) * nfixvars);
	phi_ext->nfixvars = nfixvars;
	phi_ext->phi_node = phi_node;

	size_t n;
	char gate_name[80];
	size_t offset = enter->noutputs-1;
	for (n = 0; n < nfixvars; n++) {
		snprintf(gate_name, sizeof(gate_name), "fix_%p_%zd", enter, offset+n);
		jive_gate * gate = fixvar_types[n]->create_gate(graph, gate_name);
		phi_ext->fixvars[n] = jive_node_gate_output(enter, gate);
	}

	return phi_ext;
}

struct jive_output **
jive_phi_end_extension(struct jive_phi_extension * self)
{
	jive_node * phi_node = self->phi_node;
	jive_context * context = phi_node->region->graph->context;
	jive_node * enter = jive_phi_node_get_enter_node(self->phi_node);
	jive_node * leave = jive_phi_node_get_leave_node(self->phi_node);

	size_t n;
	size_t offset = leave->ninputs;
	for (n = 0; n < self->nfixvars; n++) {
		jive_gate * gate = enter->outputs[offset+n]->gate;
		jive_node_gate_input(leave, gate, self->fixvars[n]);
		jive_node_gate_output(phi_node, gate);
	}

	jive_graph_mark_denormalized(phi_node->region->graph);

	jive_context_free(context, self->fixvars);
	jive_context_free(context, self);

	return &phi_node->outputs[offset-1];
}
