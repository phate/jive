#include <jive/vsdg/controltype.h>

#include <string.h>

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
jive_control_type_create_input_(const jive_type * self, jive_node * node, size_t index, jive_output * initial_operand);

jive_output *
jive_control_type_create_output_(const jive_type * self, jive_node * node, size_t index);

jive_gate *
jive_control_type_create_gate_(const jive_type * self, jive_graph * graph, const char * name);

void
jive_control_input_init_(jive_control_input * self, jive_node * node, size_t index, jive_output * origin);

void
jive_control_input_fini_(jive_input * self_);

const jive_type *
jive_control_input_get_type_(const jive_input * self);

void
jive_control_output_init_(jive_control_output * self, jive_node * node, size_t index);

const jive_type *
jive_control_output_get_type_(const jive_output * self);

void
jive_control_gate_init_(jive_control_gate * self, struct jive_graph * graph, const char name[]);

const jive_type *
jive_control_gate_get_type_(const jive_gate * self);

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
	.fini = jive_type_fini_, /* inherit */
	.get_label = jive_control_type_get_label_, /* override */
	.create_input = jive_control_type_create_input_, /* override */
	.create_output = jive_control_type_create_output_, /* override */
	.create_gate = jive_control_type_create_gate_, /* override */
	.equals = jive_type_equals_, /* inherit */
	.copy = jive_type_copy_ /* inherit */
};

const jive_input_class JIVE_CONTROL_INPUT = {
	.parent = &JIVE_STATE_INPUT,
	.fini = jive_control_input_fini_, /* override */
	.get_label = jive_control_input_get_label_, /* override */
	.get_type = jive_control_input_get_type_, /* override */
};

const jive_output_class JIVE_CONTROL_OUTPUT = {
	.parent = &JIVE_STATE_OUTPUT,
	.fini = jive_output_fini_, /* inherit */
	.get_label = jive_control_output_get_label_, /* override */
	.get_type = jive_control_output_get_type_, /* override */
};

const jive_gate_class JIVE_CONTROL_GATE = {
	.parent = &JIVE_STATE_GATE,
	.fini = jive_gate_fini_, /* inherit */
	.get_label = jive_control_gate_get_label_, /* override */
	.get_type = jive_control_gate_get_type_, /* override */
};

jive_input *
jive_control_type_create_input_(const jive_type * self, jive_node * node, size_t index, jive_output * initial_operand)
{
	jive_control_input * input = jive_context_malloc(node->graph->context, sizeof(*input));
	input->base.base.class_ = &JIVE_CONTROL_INPUT;
	jive_control_input_init_(input, node, index, initial_operand);
	return &input->base.base;
}

jive_output *
jive_control_type_create_output_(const jive_type * self, jive_node * node, size_t index)
{
	jive_control_output * output = jive_context_malloc(node->graph->context, sizeof(*output));
	output->base.base.class_ = &JIVE_CONTROL_OUTPUT;
	output->active = true;
	jive_control_output_init_(output, node, index);
	return &output->base.base;
}

jive_gate *
jive_control_type_create_gate_(const jive_type * self, struct jive_graph * graph, const char * name)
{
	jive_control_gate * gate = jive_context_malloc(graph->context, sizeof(*gate));
	gate->base.base.class_ = &JIVE_CONTROL_GATE;
	jive_control_gate_init_(gate, graph, name);
	return &gate->base.base;
}

void
jive_control_input_init_(jive_control_input * self, jive_node * node, size_t index, jive_output * origin)
{
	jive_state_input_init_(&self->base, node, index, origin);
}

void
jive_control_input_fini_(jive_input * self_)
{
	jive_control_input * self = (jive_control_input *)self_;
	jive_input_fini_(&self->base.base);
}

const jive_type *
jive_control_input_get_type_(const jive_input * self)
{
	return &jive_control_type_singleton.base.base;
}

void
jive_control_output_init_(jive_control_output * self, jive_node * node, size_t index)
{
	jive_state_output_init_(&self->base, node, index);
}

const jive_type *
jive_control_output_get_type_(const jive_output * self)
{
	return &jive_control_type_singleton.base.base;
}

void
jive_control_gate_init_(jive_control_gate * self, struct jive_graph * graph, const char * name)
{
	jive_state_gate_init_(&self->base, graph, name);
}

const jive_type *
jive_control_gate_get_type_(const jive_gate * self)
{
	return &jive_control_type_singleton.base.base;
}
