#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/anchortype-private.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>
#include <jive/util/list.h>
#include <jive/debug-private.h>

#include <jive/vsdg/node.h>
#include <jive/vsdg/graph.h>

static const jive_anchor_type jive_anchor_type_singleton = {
	.base = { .class_ = &JIVE_ANCHOR_TYPE }
};

const jive_type_class JIVE_ANCHOR_TYPE = {
	.parent = &JIVE_TYPE,
	.fini = _jive_type_fini, /* inherit */
	.get_label = _jive_type_get_label, /* inherit */
	.create_input = _jive_anchor_type_create_input, /* override */
	.create_output = _jive_anchor_type_create_output, /* override */
	.create_gate = _jive_type_create_gate, /* inherit */
	.equals = _jive_type_equals, /* inherit */
	.copy = _jive_type_copy /* inherit */
};

const jive_input_class JIVE_ANCHOR_INPUT = {
	.parent = &JIVE_INPUT,
	.fini = _jive_anchor_input_fini, /* override */
	.get_label = _jive_input_get_label, /* inherit */
	.get_type = _jive_anchor_input_get_type, /* override */
};

const jive_output_class JIVE_ANCHOR_OUTPUT = {
	.parent = &JIVE_OUTPUT,
	.fini = _jive_output_fini, /* inherit */
	.get_label = _jive_output_get_label, /* inherit */
	.get_type = _jive_anchor_output_get_type, /* override */
};

jive_input *
_jive_anchor_type_create_input(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	jive_anchor_input * input = jive_context_malloc(node->graph->context, sizeof(*input));
	input->base.class_ = &JIVE_ANCHOR_INPUT;
	_jive_anchor_input_init(input, node, index, initial_operand);
	return &input->base; 
}

jive_output *
_jive_anchor_type_create_output(const jive_type * self, struct jive_node * node, size_t index)
{
	jive_anchor_output * output = jive_context_malloc(node->graph->context, sizeof(*output));
	output->base.class_ = &JIVE_ANCHOR_OUTPUT;
	_jive_anchor_output_init(output, node, index);
	return &output->base; 
}

void
_jive_anchor_input_init(jive_anchor_input * self, struct jive_node * node, size_t index, jive_output * origin)
{
	_jive_input_init(&self->base, node, index, origin);
	DEBUG_ASSERT(origin->node->region->anchor == 0);
	origin->node->region->anchor = &self->base;
}

void
_jive_anchor_input_fini(jive_input * self_)
{
	jive_anchor_input * self = (jive_anchor_input *)self_;
	if (self->base.origin->node->region->anchor == &self->base) {
		self->base.origin->node->region->anchor = 0;
	}
	_jive_input_fini(&self->base);
}

const jive_type *
_jive_anchor_input_get_type(const jive_input * self)
{
	return &jive_anchor_type_singleton.base;
}

void
_jive_anchor_output_init(jive_anchor_output * self, struct jive_node * node, size_t index)
{
	_jive_output_init(&self->base, node, index);
}

const jive_type *
_jive_anchor_output_get_type(const jive_output * self)
{
	return &jive_anchor_type_singleton.base;
}
