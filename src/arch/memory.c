#include <jive/arch/memory.h>

#include <string.h>

#include <jive/context.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/statetype-private.h>

const jive_memory_type jive_memory_type_singleton = {
	.base = { .base = { .class_ = &JIVE_MEMORY_TYPE } }
};

static void
jive_memory_input_init_(jive_memory_input * self, struct jive_node * node, size_t index, jive_output * origin)
{
	_jive_state_input_init(&self->base, node, index, origin);
}

static const jive_type *
jive_memory_input_get_type_(const jive_input * self)
{
	return &jive_memory_type_singleton.base.base;
}

static void
jive_memory_output_init_(jive_memory_output * self, struct jive_node * node, size_t index)
{
	_jive_state_output_init(&self->base, node, index);
}

static const jive_type *
jive_memory_output_get_type_(const jive_output * self)
{
	return &jive_memory_type_singleton.base.base;
}

void
jive_memory_gate_init_(jive_memory_gate * self, struct jive_graph * graph, const char * name)
{
	_jive_state_gate_init(&self->base, graph, name);
}

const jive_type *
jive_memory_gate_get_type_(const jive_gate * self)
{
	return &jive_memory_type_singleton.base.base;
}

static jive_type *
jive_memory_type_copy_(const jive_type * self_, jive_context * context)
{
	const jive_memory_type * self = (const jive_memory_type *) self_;
	
	jive_memory_type * type = jive_context_malloc(context, sizeof(*type));
	
	type->base = self->base;
	
	return &type->base.base;
}

static char *
jive_memory_type_get_label_(const jive_type * self)
{
	return strdup("mem");
}

static jive_input *
jive_memory_type_create_input_(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	jive_memory_input * input = jive_context_malloc(node->graph->context, sizeof(*input));
	input->base.base.class_ = &JIVE_MEMORY_INPUT;
	jive_memory_input_init_(input, node, index, initial_operand);
	return &input->base.base;
}

static jive_output *
jive_memory_type_create_output_(const jive_type * self, struct jive_node * node, size_t index)
{
	jive_memory_output * output = jive_context_malloc(node->graph->context, sizeof(*output));
	output->base.base.class_ = &JIVE_MEMORY_OUTPUT;
	jive_memory_output_init_(output, node, index);
	return &output->base.base;
}

static jive_gate *
jive_memory_type_create_gate_(const jive_type * self, struct jive_graph * graph, const char * name)
{
	jive_memory_gate * gate = jive_context_malloc(graph->context, sizeof(*gate));
	gate->base.base.class_ = &JIVE_MEMORY_GATE;
	jive_memory_gate_init_(gate, graph, name);
	return &gate->base.base;
}

const jive_type_class JIVE_MEMORY_TYPE = {
	.parent = &JIVE_TYPE,
	.fini = _jive_state_type_fini, /* inherit */
	.copy = jive_memory_type_copy_, /* override */
	.get_label = jive_memory_type_get_label_, /* inherit */
	.create_input = jive_memory_type_create_input_, /* override */
	.create_output = jive_memory_type_create_output_, /* override */
	.create_gate = jive_memory_type_create_gate_, /* override */
	.equals = _jive_type_equals, /* inherit */
};

const jive_input_class JIVE_MEMORY_INPUT = {
	.parent = &JIVE_INPUT,
	.fini = _jive_input_fini, /* inherit */
	.get_label = _jive_input_get_label, /* inherit */
	.get_type = jive_memory_input_get_type_, /* override */
};

const jive_output_class JIVE_MEMORY_OUTPUT = {
	.parent = &JIVE_OUTPUT,
	.fini = _jive_output_fini, /* inherit */
	.get_label = _jive_output_get_label, /* inherit */
	.get_type = jive_memory_output_get_type_, /* override */
};

const jive_gate_class JIVE_MEMORY_GATE = {
	.parent = &JIVE_GATE,
	.fini = _jive_gate_fini, /* inherit */
	.get_label = _jive_gate_get_label, /* inherit */
	.get_type = jive_memory_gate_get_type_, /* override */
};

