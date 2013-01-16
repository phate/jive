/*
 * Copyright 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/phi.h>

#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/valuetype.h>
#include <jive/vsdg/substitution.h>

/* phi node normal form */

bool
jive_phi_node_normal_form_normalize_node_(const jive_node_normal_form * self_, jive_node * node)
{
	const jive_phi_node_normal_form * self = (const jive_phi_node_normal_form *) self_;
	if (!self->base.enable_mutable)
		return true;

	const jive_node_attrs * attrs = jive_node_get_attrs(node);

	if (self->base.enable_cse) {
		jive_node * new_node = jive_node_cse(node->region->graph, self->base.node_class, attrs, 1,
			&node->inputs[0]->origin);
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

bool
jive_phi_node_normal_form_operands_are_normalized_(const jive_node_normal_form * self_,
	size_t noperands, jive_output * const operands[], const jive_node_attrs * attrs)
{
	const jive_phi_node_normal_form * self = (const jive_phi_node_normal_form *) self_;
	if (!self->base.enable_mutable)
		return true;

	JIVE_DEBUG_ASSERT(noperands == 1);

	jive_graph * graph = operands[0]->node->graph;
	const jive_node_class * cls = self->base.node_class;

	if (self->base.enable_cse && jive_node_cse(graph, cls, attrs, noperands, operands))
		return false;

	return true;
}

void
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
		jive_node * node = jive_node_cse(phi_region->graph, cls, NULL, 1, &operand);
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
	.base = { /* jive_anchor_node_normal_form_class */
		.base = { /* jive_node_normal_form_class */
			.parent = &JIVE_ANCHOR_NODE_NORMAL_FORM,
			.fini = jive_node_normal_form_fini_, /* inherit */
			.normalize_node = jive_phi_node_normal_form_normalize_node_, /* override */
			.operands_are_normalized = jive_phi_node_normal_form_operands_are_normalized_, /* override */
			
			.set_mutable = jive_node_normal_form_set_mutable_, /* inherit */
			.set_cse = jive_node_normal_form_set_cse_ /* inherit */
		},
		.set_reducible = jive_anchor_node_normal_form_set_reducible_ /* inherit */
	},
	.normalized_create= jive_phi_node_normalized_create_
};

/* phi enter node */

static jive_node *
jive_phi_enter_node_create(jive_region * region)
{
	JIVE_DEBUG_ASSERT(region->top == NULL && region->bottom == NULL);
	jive_node * node = jive_context_malloc(region->graph->context, sizeof(*node));

	node->class_ = &JIVE_PHI_ENTER_NODE;
	JIVE_DECLARE_CONTROL_TYPE(ctltype);
	jive_node_init_(node, region,
		0, NULL, NULL,
		1, &ctltype);
	((jive_control_output *)node->outputs[0])->active = false;
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
	.parent = &JIVE_NODE,
	.name = "PHI_ENTER",
	.fini = jive_node_fini_, /* inherit */
	.get_default_normal_form = jive_node_get_default_normal_form_, /* inherit */
	.get_label = jive_node_get_label_, /* inherit */
	.get_attrs = jive_node_get_attrs_, /* inherit */
	.match_attrs = jive_node_match_attrs_, /* inherit */
	.create = jive_phi_enter_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

/* phi leave node */

static jive_node *
jive_phi_leave_node_create(jive_output * output)
{
	jive_node * node = jive_context_malloc(output->node->graph->context, sizeof(*node));

	node->class_ = &JIVE_PHI_LEAVE_NODE;
	JIVE_DECLARE_CONTROL_TYPE(ctltype);
	JIVE_DECLARE_ANCHOR_TYPE(anctype);
	jive_node_init_(node, output->node->region,
		1, &ctltype, &output,
		1, &anctype);

	JIVE_DEBUG_ASSERT(output->node->region->bottom == NULL);
	output->node->region->bottom = node;

	return node;
}

static jive_node *
jive_phi_leave_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);
	return jive_phi_leave_node_create(operands[0]);
}

const jive_node_class JIVE_PHI_LEAVE_NODE = {
	.parent = &JIVE_NODE,
	.name = "PHI_LEAVE",
	.fini = jive_node_fini_, /* inherit */
	.get_default_normal_form = jive_node_get_default_normal_form_, /* inherit */
	.get_label = jive_node_get_label_, /* inherit */
	.get_attrs = jive_node_get_attrs_, /* inherit */
	.match_attrs = jive_node_match_attrs_, /* inherit */
	.create = jive_phi_leave_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

/* phi node */

static void
jive_phi_node_fini_(jive_node * self_);

static jive_node_normal_form *
jive_phi_node_get_default_normal_form_(const jive_node_class * cls,
	jive_node_normal_form * parent_, struct jive_graph * graph);

static const jive_node_attrs *
jive_phi_node_get_attrs_(const jive_node * self);

static bool
jive_phi_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs);

static jive_node *
jive_phi_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_PHI_NODE = {
	.parent = &JIVE_NODE,
	.name = "PHI",
	.fini = jive_phi_node_fini_, /* override */
	.get_default_normal_form = jive_phi_node_get_default_normal_form_, /* override */
	.get_label = jive_node_get_label_, /* inherit */
	.get_attrs = jive_phi_node_get_attrs_, /* inherit */
	.match_attrs = jive_phi_node_match_attrs_, /* override */
	.create = jive_phi_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

static void
jive_phi_node_init_(jive_phi_node * self, jive_region * phi_region)
{
	jive_region * region = phi_region->parent;
	jive_context * context = phi_region->graph->context;
	jive_node * enter = jive_region_get_top_node(phi_region);
	jive_node * leave = jive_region_get_bottom_node(phi_region);

	size_t narguments = enter->noutputs - 1;
	size_t nreturns = leave->ninputs - 1;

	if (narguments != nreturns)
		jive_context_fatal_error(context, "Type mismatch: inputs and outputs need to have the same arity");

	size_t n;
	for (n = 0; n < nreturns; n++) {
		if (enter->outputs[n+1]->gate != leave->inputs[n+1]->gate)
			jive_context_fatal_error(context, "Type mismatch: input and output must share the same gate");
		
	}

	JIVE_DECLARE_ANCHOR_TYPE(anctype);
	jive_node_init_(&self->base, region,
		1, &anctype, &phi_region->bottom->outputs[0],
		0, NULL);

	self->attrs.gates = jive_context_malloc(context, narguments * sizeof(jive_gate *));
	for(n = 0; n < narguments; n++) {
		jive_gate * gate = enter->outputs[n+1]->gate;
		jive_node_gate_output(&self->base, gate);
		self->attrs.gates[n] = gate; 
	}
}

static void
jive_phi_node_fini_(jive_node * self_)
{
	jive_context * context = self_->graph->context;
	jive_phi_node * self = (jive_phi_node *) self_;
	jive_context_free(context, self->attrs.gates);

	jive_node_fini_(&self->base);
}

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
	
	jive_phi_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->base.class_ = &JIVE_PHI_NODE;
	jive_phi_node_init_(node, operands[0]->node->region);

	return &node->base; 
}

static const jive_node_attrs *
jive_phi_node_get_attrs_(const jive_node * self_)
{
	const jive_phi_node * self = (const jive_phi_node *) self_;

	return &self->attrs.base;
}

static bool
jive_phi_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_phi_node_attrs * first = &((const jive_phi_node *)self)->attrs;
	const jive_phi_node_attrs * second = (const jive_phi_node_attrs *) attrs;

	size_t n;
	for(n = 0; n < self->noutputs; n++)
		if (first->gates[n] != second->gates[n])
			return false;

	return true;
}

struct jive_region *
jive_phi_region_create(struct jive_region * parent,
	size_t narguments, const struct jive_type * argument_types[], struct jive_output * arguments[])
{
	jive_region * phi_region = jive_region_create_subregion(parent);
	jive_node * enter = jive_phi_enter_node_create(phi_region);
	jive_phi_leave_node_create(enter->outputs[0]);

	size_t n;
	for (n = 0; n < narguments; n++) {
		jive_gate * gate = jive_type_create_gate(argument_types[n], parent->graph, "");
		arguments[n] = jive_node_gate_output(enter, gate);
	}

	return phi_region;
}

struct jive_output *
jive_phi_region_finalize(struct jive_region * phi_region,
	size_t nreturns, jive_output * returns[])
{
	size_t n;
	jive_node * enter = jive_region_get_top_node(phi_region);
	jive_node * leave = jive_region_get_bottom_node(phi_region);
	
	if (enter->noutputs-1 != nreturns)
		jive_context_fatal_error(enter->region->graph->context, "Type mismatch: inputs and outputs need to have the same arity");
	
	for(n = 0; n < nreturns; n++)
		jive_node_gate_input(leave, enter->outputs[n+1]->gate, returns[n]);

	return leave->outputs[0];
}

void
jive_phi_create(jive_region * phi_region, jive_output * results[])
{
	const jive_phi_node_normal_form * nf = (const jive_phi_node_normal_form *)
		jive_graph_get_nodeclass_form(phi_region->graph, &JIVE_PHI_NODE);

	const jive_phi_node_normal_form_class * cls;
	cls = (const jive_phi_node_normal_form_class *) nf->base.class_;

	return cls->normalized_create(nf, phi_region, results);
}
