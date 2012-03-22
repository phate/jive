#include <jive/vsdg/control.h>

#include <string.h>
#include <stdio.h>

#include <jive/common.h>
#include <jive/context.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

static jive_node *
jive_theta_head_node_create_(jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[])
{
	jive_node * self = jive_context_malloc(region->graph->context, sizeof(*self));
	JIVE_DECLARE_CONTROL_TYPE(control);
	self->class_ = &JIVE_THETA_HEAD_NODE;
	jive_node_init_(self, region,
		0, NULL, NULL,
		1, &control);
	
	region->top = self;
	return self;
}

static jive_node *
jive_theta_tail_node_create_(jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);
	jive_node * self = jive_context_malloc(region->graph->context, sizeof(*self));
	JIVE_DECLARE_ANCHOR_TYPE(anchor);
	JIVE_DECLARE_CONTROL_TYPE(control);
	self->class_ = &JIVE_THETA_TAIL_NODE;
	jive_node_init_(self, region,
		1, &control, operands,
		1, &anchor);
	
	region->bottom = self;
	return self;
}

static jive_node *
jive_theta_node_create_(jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);
	jive_node * self = jive_context_malloc(region->graph->context, sizeof(*self));;
	JIVE_DECLARE_ANCHOR_TYPE(anchor);
	self->class_ = &JIVE_THETA_NODE;
	jive_node_init_(self, region,
		1, &anchor, operands,
		0, NULL);
	
	return self;
}

const jive_node_class JIVE_THETA_HEAD_NODE = {
	.parent = &JIVE_NODE,
	.name = "THETA_HEAD",
	.fini = jive_node_fini_,  /* inherit */
	.get_default_normal_form = jive_node_get_default_normal_form_,  /* inherit */
	.get_label = jive_node_get_label_,  /* inherit */
	.get_attrs = jive_node_get_attrs_,  /* inherit */
	.match_attrs = jive_node_match_attrs_,  /* inherit */
	.create = jive_theta_head_node_create_,  /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_  /* inherit */
};

const jive_node_class JIVE_THETA_TAIL_NODE = {
	.parent = &JIVE_NODE,
	.name = "THETA_TAIL",
	.fini = jive_node_fini_,  /* inherit */
	.get_default_normal_form = jive_node_get_default_normal_form_,  /* inherit */
	.get_label = jive_node_get_label_,  /* inherit */
	.get_attrs = jive_node_get_attrs_,  /* inherit */
	.match_attrs = jive_node_match_attrs_,  /* inherit */
	.create = jive_theta_tail_node_create_,  /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_  /* inherit */
};

const jive_node_class JIVE_THETA_NODE = {
	.parent = &JIVE_NODE,
	.name = "THETA",
	.fini = jive_node_fini_,  /* inherit */
	.get_default_normal_form = jive_node_get_default_normal_form_,  /* inherit */
	.get_label = jive_node_get_label_,  /* inherit */
	.get_attrs = jive_node_get_attrs_,  /* inherit */
	.match_attrs = jive_node_match_attrs_,  /* inherit */
	.create = jive_theta_node_create_,  /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_  /* inherit */
};

static jive_node *
jive_theta_head_node_create(jive_region * region)
{
	jive_node * self = jive_context_malloc(region->graph->context, sizeof(*self));
	JIVE_DECLARE_CONTROL_TYPE(control);
	self->class_ = &JIVE_THETA_HEAD_NODE;
	jive_node_init_(self, region,
		0, NULL, NULL,
		1, &control);
	
	region->top = self;
	return self;
}

static jive_node *
jive_theta_tail_node_create(jive_region * region, jive_output * predicate)
{
	jive_node * self = jive_context_malloc(region->graph->context, sizeof(*self));
	JIVE_DECLARE_ANCHOR_TYPE(anchor);
	JIVE_DECLARE_CONTROL_TYPE(control);
	self->class_ = &JIVE_THETA_TAIL_NODE;
	jive_node_init_(self, region,
		1, &control, &predicate,
		1, &anchor);
	
	region->bottom = self;
	return self;
}

static jive_node *
jive_theta_node_create(jive_region * region,
	jive_output * loop_body)
{
	jive_node * self = jive_context_malloc(region->graph->context, sizeof(*self));;
	JIVE_DECLARE_ANCHOR_TYPE(anchor);
	self->class_ = &JIVE_THETA_NODE;
	jive_node_init_(self, region,
		1, &anchor, &loop_body,
		0, NULL);
	
	return self;
}

jive_node *
jive_theta_create(
	jive_region * region,
	size_t nvalues, const jive_type * types[const], jive_output * values[const])
{
	jive_region * loop_region = jive_region_create_subregion(region);
	jive_node * head = jive_theta_head_node_create(loop_region);
	jive_node * tail = jive_theta_tail_node_create(loop_region, head->outputs[0]);
	jive_node * theta = jive_theta_node_create(region, tail->outputs[0]);
	
	size_t n;
	for (n = 0; n < nvalues; n++) {
		char name[80];
		snprintf(name, sizeof(name), "theta_%p_%zd", theta, n);
		jive_gate * gate = jive_type_create_gate(types[n], region->graph, name);
		jive_node_gate_input(head, gate, values[n]);
		jive_output * inner = jive_node_gate_output(head, gate);
		jive_node_gate_input(tail, gate, inner);
		jive_node_gate_output(theta, gate);
	}
	return theta;
}

jive_node *
jive_control_false_create(jive_region * region)
{
	jive_node * self = jive_context_malloc(region->graph->context, sizeof(*self));
	JIVE_DECLARE_CONTROL_TYPE(control);
	self->class_ = &JIVE_CONTROL_FALSE_NODE;
	jive_node_init_(self, region,
		0, NULL, NULL,
		1, &control);
	
	return self;
}

jive_node *
jive_control_true_create(jive_region * region)
{
	jive_node * self = jive_context_malloc(region->graph->context, sizeof(*self));
	JIVE_DECLARE_CONTROL_TYPE(control);
	self->class_ = &JIVE_CONTROL_TRUE_NODE;
	jive_node_init_(self, region,
		0, NULL, NULL,
		1, &control);
	
	return self;
}

jive_output *
jive_control_false(jive_graph * graph)
{
	return jive_control_false_create(graph->root_region)->outputs[0];
}

jive_output *
jive_control_true(jive_graph * graph)
{
	return jive_control_true_create(graph->root_region)->outputs[0];
}

static jive_node *
jive_control_false_node_create_(jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 0);
	
	return jive_control_false_create(region);
}

static jive_node *
jive_control_true_node_create_(jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 0);
	
	return jive_control_true_create(region);
}


const jive_node_class JIVE_CONTROL_FALSE_NODE = {
	.parent = &JIVE_NODE,
	.name = "FALSE",
	.fini = jive_node_fini_,  /* inherit */
	.get_default_normal_form = jive_node_get_default_normal_form_,  /* inherit */
	.get_label = jive_node_get_label_,  /* inherit */
	.get_attrs = jive_node_get_attrs_,  /* inherit */
	.match_attrs = jive_node_match_attrs_,  /* inherit */
	.create = jive_control_false_node_create_,  /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_  /* inherit */
};

const jive_node_class JIVE_CONTROL_TRUE_NODE = {
	.parent = &JIVE_NODE,
	.name = "TRUE",
	.fini = jive_node_fini_,  /* inherit */
	.get_default_normal_form = jive_node_get_default_normal_form_,  /* inherit */
	.get_label = jive_node_get_label_,  /* inherit */
	.get_attrs = jive_node_get_attrs_,  /* inherit */
	.match_attrs = jive_node_match_attrs_,  /* inherit */
	.create = jive_control_true_node_create_,  /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_  /* inherit */
};
