#include <jive/vsdg/statetype.h>
#include <jive/vsdg/statetype-private.h>
#include <jive/vsdg/basetype-private.h>

#include <jive/vsdg/node.h>
#include <jive/vsdg/graph.h>

const jive_type_class JIVE_STATE_TYPE = {
	.parent = &JIVE_TYPE,
	.get_label = _jive_type_get_label, /* inherit */
	.create_input = _jive_state_type_create_input, /* override */
	.create_output = _jive_state_type_create_output, /* override */
	.create_resource = _jive_state_type_create_resource, /* override */
	.create_gate = _jive_state_type_create_gate, /* override */
	.equals = _jive_type_equals, /* inherit */
	.accepts = _jive_type_accepts /* inherit */
};

const jive_type jive_state_type_singleton = {
	.class_ = &JIVE_STATE_TYPE
};

const jive_input_class JIVE_STATE_INPUT = {
	.parent = &JIVE_INPUT,
	.fini = _jive_input_fini, /* inherit */
	.get_label = _jive_input_get_label, /* inherit */
	.get_type = _jive_state_input_get_type, /* override */
	.get_constraint = _jive_input_get_constraint /* inherit */
};

const jive_output_class JIVE_STATE_OUTPUT = {
	.parent = &JIVE_OUTPUT,
	.fini = _jive_output_fini, /* inherit */
	.get_label = _jive_output_get_label, /* inherit */
	.get_type = _jive_state_output_get_type, /* override */
	.get_constraint = _jive_output_get_constraint /* inherit */
};

const jive_gate_class JIVE_STATE_GATE = {
	.parent = &JIVE_GATE,
	.fini = _jive_gate_fini, /* inherit */
	.get_label = _jive_gate_get_label, /* inherit */
	.get_type = _jive_state_gate_get_type /* override */
};

const jive_resource_class JIVE_STATE_RESOURCE = {
	.parent = &JIVE_RESOURCE,
	.fini = _jive_resource_fini, /* inherit */
	.get_label = _jive_resource_get_label, /* inherit */
	.get_type = _jive_state_resource_get_type, /* override */
	.can_merge = _jive_resource_can_merge, /* inherit */
	.merge = _jive_resource_merge, /* inherit */
	.get_cpureg = _jive_resource_get_cpureg, /* inherit */
	.get_regcls = _jive_resource_get_regcls, /* inherit */
	.get_real_regcls = _jive_resource_get_real_regcls /* inherit */
};

jive_input *
_jive_state_type_create_input(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	jive_state_input * input = jive_context_malloc(node->graph->context, sizeof(*input));
	input->base.class_ = &JIVE_STATE_INPUT;
	_jive_state_input_init(input, node, index, initial_operand);
	return &input->base; 
}

jive_output *
_jive_state_type_create_output(const jive_type * self, struct jive_node * node, size_t index)
{
	jive_state_output * output = jive_context_malloc(node->graph->context, sizeof(*output));
	output->base.class_ = &JIVE_STATE_OUTPUT;
	_jive_state_output_init(output, node, index);
	return &output->base; 
}

jive_resource *
_jive_state_type_create_resource(const jive_type * self, struct jive_graph * graph)
{
	jive_state_resource * resource = jive_context_malloc(graph->context, sizeof(*resource));
	resource->base.class_ = &JIVE_STATE_RESOURCE;
	_jive_state_resource_init(resource);
	return &resource->base; 
}

jive_gate *
_jive_state_type_create_gate(const jive_type * self, struct jive_graph * graph, const char * name)
{
	jive_state_gate * gate = jive_context_malloc(graph->context, sizeof(*gate));
	gate->base.class_ = &JIVE_STATE_GATE;
	_jive_state_gate_init(gate, graph, name);
	return &gate->base; 
}

void
_jive_state_input_init(jive_state_input * self, struct jive_node * node, size_t index, jive_output * origin)
{
	_jive_input_init(&self->base, node, index, origin);
}

const jive_type *
_jive_state_input_get_type(const jive_input * self)
{
	return &jive_state_type_singleton;
}

void
_jive_state_output_init(jive_state_output * self, struct jive_node * node, size_t index)
{
	_jive_output_init(&self->base, node, index);
}

const jive_type *
_jive_state_output_get_type(const jive_output * self)
{
	return &jive_state_type_singleton;
}

void
_jive_state_resource_init(jive_state_resource * self)
{
	_jive_resource_init(&self->base);
}

const jive_type *
_jive_state_resource_get_type(const jive_resource * self)
{
	return &jive_state_type_singleton;
}

void
_jive_state_gate_init(jive_state_gate * self, struct jive_graph * graph, const char * name)
{
	_jive_gate_init(&self->base, graph, name);
}

const jive_type *
_jive_state_gate_get_type(const jive_gate * self)
{
	return &jive_state_type_singleton;
}

