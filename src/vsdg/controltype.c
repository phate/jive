#include <jive/vsdg/controltype.h>

#include <string.h>

#include <jive/debug-private.h>
#include <jive/util/list.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/statetype-private.h>

static const jive_control_type jive_control_type_singleton = {
	.base = { .base = { .class_ = &JIVE_CONTROL_TYPE } }
};

jive_input *
_jive_control_type_create_input(const jive_type * self, jive_node * node, size_t index, jive_output * initial_operand);

jive_output *
_jive_control_type_create_output(const jive_type * self, jive_node * node, size_t index);

jive_gate *
_jive_control_type_create_gate(const jive_type * self, jive_graph * graph, const char * name);

void
_jive_control_input_init(jive_control_input * self, jive_node * node, size_t index, jive_output * origin);

void
_jive_control_input_fini(jive_input * self_);

const jive_type *
_jive_control_input_get_type(const jive_input * self);

void
_jive_control_output_init(jive_control_output * self, jive_node * node, size_t index);

const jive_type *
_jive_control_output_get_type(const jive_output * self);

void
_jive_control_gate_init(jive_control_gate * self, struct jive_graph * graph, const char name[]);

const jive_type *
_jive_control_gate_get_type(const jive_gate * self);

static char *
jive_control_type_get_label_(const jive_type * type)
{
	return strdup("ctl");
};

static char *
jive_control_input_get_label_(const jive_input * input)
{
	return strdup("ctl");
};

static char *
jive_control_output_get_label_(const jive_output * output)
{
	return strdup("ctl");
};

static char *
jive_control_gate_get_label_(const jive_gate * gate)
{
	return strdup("ctl");
};

const jive_type_class JIVE_CONTROL_TYPE = {
	.parent = &JIVE_STATE_TYPE,
	.fini = _jive_type_fini, /* inherit */
	.get_label = jive_control_type_get_label_, /* override */
	.create_input = _jive_control_type_create_input, /* override */
	.create_output = _jive_control_type_create_output, /* override */
	.create_gate = _jive_type_create_gate, /* inherit */
	.equals = _jive_type_equals, /* inherit */
	.copy = _jive_type_copy /* inherit */
};

const jive_input_class JIVE_CONTROL_INPUT = {
	.parent = &JIVE_STATE_INPUT,
	.fini = _jive_control_input_fini, /* override */
	.get_label = jive_control_input_get_label_, /* override */
	.get_type = _jive_control_input_get_type, /* override */
};

const jive_output_class JIVE_CONTROL_OUTPUT = {
	.parent = &JIVE_STATE_OUTPUT,
	.fini = _jive_output_fini, /* inherit */
	.get_label = jive_control_output_get_label_, /* override */
	.get_type = _jive_control_output_get_type, /* override */
};

const jive_gate_class JIVE_CONTROL_GATE = {
	.parent = &JIVE_STATE_GATE,
	.fini = _jive_gate_fini, /* inherit */
	.get_label = jive_control_gate_get_label_, /* override */
	.get_type = _jive_control_gate_get_type, /* override */
};

jive_input *
_jive_control_type_create_input(const jive_type * self, jive_node * node, size_t index, jive_output * initial_operand)
{
	jive_control_input * input = jive_context_malloc(node->graph->context, sizeof(*input));
	input->base.base.class_ = &JIVE_CONTROL_INPUT;
	_jive_control_input_init(input, node, index, initial_operand);
	return &input->base.base;
}

jive_output *
_jive_control_type_create_output(const jive_type * self, jive_node * node, size_t index)
{
	jive_control_output * output = jive_context_malloc(node->graph->context, sizeof(*output));
	output->base.base.class_ = &JIVE_CONTROL_OUTPUT;
	_jive_control_output_init(output, node, index);
	return &output->base.base;
}

jive_gate *
_jive_control_type_create_gate(const jive_type * self, struct jive_graph * graph, const char * name)
{
	jive_control_gate * gate = jive_context_malloc(graph->context, sizeof(*gate));
	gate->base.base.class_ = &JIVE_CONTROL_GATE;
	_jive_control_gate_init(gate, graph, name);
	return &gate->base.base;
}

void
_jive_control_input_init(jive_control_input * self, jive_node * node, size_t index, jive_output * origin)
{
	_jive_state_input_init(&self->base, node, index, origin);
}

void
_jive_control_input_fini(jive_input * self_)
{
	jive_control_input * self = (jive_control_input *)self_;
	_jive_input_fini(&self->base.base);
}

const jive_type *
_jive_control_input_get_type(const jive_input * self)
{
	return &jive_control_type_singleton.base.base;
}

void
_jive_control_output_init(jive_control_output * self, jive_node * node, size_t index)
{
	_jive_state_output_init(&self->base, node, index);
}

const jive_type *
_jive_control_output_get_type(const jive_output * self)
{
	return &jive_control_type_singleton.base.base;
}

void
_jive_control_gate_init(jive_control_gate * self, struct jive_graph * graph, const char * name)
{
	_jive_state_gate_init(&self->base, graph, name);
}

const jive_type *
_jive_control_gate_get_type(const jive_gate * self)
{
	return &jive_control_type_singleton.base.base;
}
