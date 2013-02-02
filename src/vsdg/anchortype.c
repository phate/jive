/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/anchortype-private.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>
#include <jive/util/list.h>

#include <jive/vsdg/node.h>
#include <jive/vsdg/graph.h>

static const jive_anchor_type jive_anchor_type_singleton = {
	.base = { .class_ = &JIVE_ANCHOR_TYPE }
};

const jive_type_class JIVE_ANCHOR_TYPE = {
	.parent = &JIVE_TYPE,
	.name = "X",
	.fini = jive_type_fini_, /* inherit */
	.get_label = jive_type_get_label_, /* inherit */
	.create_input = jive_anchor_type_create_input_, /* override */
	.create_output = jive_anchor_type_create_output_, /* override */
	.create_gate = jive_type_create_gate_, /* inherit */
	.equals = jive_type_equals_, /* inherit */
	.copy = jive_type_copy_ /* inherit */
};

const jive_input_class JIVE_ANCHOR_INPUT = {
	.parent = &JIVE_INPUT,
	.fini = jive_anchor_input_fini_, /* override */
	.get_label = jive_input_get_label_, /* inherit */
	.get_type = jive_anchor_input_get_type_, /* override */
};

const jive_output_class JIVE_ANCHOR_OUTPUT = {
	.parent = &JIVE_OUTPUT,
	.fini = jive_output_fini_, /* inherit */
	.get_label = jive_output_get_label_, /* inherit */
	.get_type = jive_anchor_output_get_type_, /* override */
};

jive_input *
jive_anchor_type_create_input_(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	jive_anchor_input * input = jive_context_malloc(node->graph->context, sizeof(*input));
	input->base.class_ = &JIVE_ANCHOR_INPUT;
	jive_anchor_input_init_(input, node, index, initial_operand);
	return &input->base; 
}

jive_output *
jive_anchor_type_create_output_(const jive_type * self, struct jive_node * node, size_t index)
{
	jive_anchor_output * output = jive_context_malloc(node->graph->context, sizeof(*output));
	output->base.class_ = &JIVE_ANCHOR_OUTPUT;
	jive_anchor_output_init_(output, node, index);
	return &output->base; 
}

void
jive_anchor_input_init_(jive_anchor_input * self, struct jive_node * node, size_t index, jive_output * origin)
{
	jive_input_init_(&self->base, node, index, origin);
	JIVE_DEBUG_ASSERT(origin->node->region->anchor == 0);
	origin->node->region->anchor = &self->base;
}

void
jive_anchor_input_fini_(jive_input * self_)
{
	jive_anchor_input * self = (jive_anchor_input *)self_;
	if (self->base.origin->node->region->anchor == &self->base) {
		self->base.origin->node->region->anchor = 0;
	}
	jive_input_fini_(&self->base);
}

const jive_type *
jive_anchor_input_get_type_(const jive_input * self)
{
	return &jive_anchor_type_singleton.base;
}

void
jive_anchor_output_init_(jive_anchor_output * self, struct jive_node * node, size_t index)
{
	jive_output_init_(&self->base, node, index);
}

const jive_type *
jive_anchor_output_get_type_(const jive_output * self)
{
	return &jive_anchor_type_singleton.base;
}
