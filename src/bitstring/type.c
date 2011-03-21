#include <jive/bitstring/type-private.h>
#include <jive/vsdg/valuetype-private.h>
#include <jive/vsdg/basetype-private.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

#include <stdio.h>
#include <string.h>

const jive_type_class JIVE_BITSTRING_TYPE = {
	.parent = &JIVE_VALUE_TYPE,
	.fini = _jive_bitstring_type_fini, /* override */
	.get_label = _jive_bitstring_type_get_label, /* override */
	.create_input = _jive_bitstring_type_create_input, /* override */
	.create_output = _jive_bitstring_type_create_output, /* override */
	.create_gate = _jive_bitstring_type_create_gate, /* override */
	.equals = _jive_type_equals, /* inherit */
	.copy = _jive_bitstring_type_copy, /* override */
};

const jive_input_class JIVE_BITSTRING_INPUT = {
	.parent = &JIVE_VALUE_INPUT,
	.fini = _jive_input_fini, /* inherit */
	.get_label = _jive_input_get_label, /* inherit */
	.get_type = _jive_bitstring_input_get_type, /* override */
};

const jive_output_class JIVE_BITSTRING_OUTPUT = {
	.parent = &JIVE_VALUE_OUTPUT,
	.fini = _jive_output_fini, /* inherit */
	.get_label = _jive_output_get_label, /* inherit */
	.get_type = _jive_bitstring_output_get_type, /* override */
};

const jive_gate_class JIVE_BITSTRING_GATE = {
	.parent = &JIVE_VALUE_GATE,
	.fini = _jive_gate_fini, /* inherit */
	.get_label = _jive_gate_get_label, /* inherit */
	.get_type = _jive_bitstring_gate_get_type, /* override */
};

/* bitstring_type inheritable members */

void
_jive_bitstring_type_fini( jive_type* self_ )
{
	jive_bitstring_type* self = (jive_bitstring_type*) self_ ;

	_jive_value_type_fini( (jive_type*)&self->base ) ;
}

char *
_jive_bitstring_type_get_label(const jive_type * self_)
{
	const jive_bitstring_type * self = (const jive_bitstring_type *) self_;
	char tmp[16];
	snprintf(tmp, sizeof(tmp), "bits%zd", self->nbits);
	return strdup(tmp);
}

jive_input *
_jive_bitstring_type_create_input(const jive_type * self_, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	const jive_bitstring_type * self = (const jive_bitstring_type *) self_;
	jive_bitstring_input * input = jive_context_malloc(node->graph->context, sizeof(*input));
	input->base.base.class_ = &JIVE_BITSTRING_INPUT;
	_jive_bitstring_input_init(input, self->nbits, node, index, initial_operand);
	return &input->base.base;
}

jive_output *
_jive_bitstring_type_create_output(const jive_type * self_, struct jive_node * node, size_t index)
{
	const jive_bitstring_type * self = (const jive_bitstring_type *) self_;
	jive_bitstring_output * output = jive_context_malloc(node->graph->context, sizeof(*output));
	output->base.base.class_ = &JIVE_BITSTRING_OUTPUT;
	_jive_bitstring_output_init(output, self->nbits, node, index);
	return &output->base.base;
}

jive_gate *
_jive_bitstring_type_create_gate(const jive_type * self_, struct jive_graph * graph, const char * name)
{
	const jive_bitstring_type * self = (const jive_bitstring_type *) self_;
	jive_bitstring_gate * gate = jive_context_malloc(graph->context, sizeof(*gate));
	gate->base.base.class_ = &JIVE_BITSTRING_GATE;
	_jive_bitstring_gate_init(gate, self->nbits, graph, name);
	return &gate->base.base;
}

jive_type *
_jive_bitstring_type_copy(const jive_type * self_, jive_context * context)
{
	const jive_bitstring_type * self = (const jive_bitstring_type *) self_;
	
	jive_bitstring_type * type = jive_context_malloc(context, sizeof(*type));
	
	*type = *self;
	
	return &type->base.base;
}

static inline void
_jive_bitstring_type_init(jive_bitstring_type * self, size_t nbits)
{
	self->base.base.class_ = &JIVE_BITSTRING_TYPE;
	self->nbits = nbits;
}

/* bitstring_input inheritable members */

void
_jive_bitstring_input_init(jive_bitstring_input * self, size_t nbits, struct jive_node * node, size_t index, jive_output * origin)
{
	_jive_value_input_init(&self->base, node, index, origin);
	_jive_bitstring_type_init(&self->type, nbits);
}

const jive_type *
_jive_bitstring_input_get_type(const jive_input * self_)
{
	const jive_bitstring_input * self = (const jive_bitstring_input *) self_;
	return &self->type.base.base;
}

/* bitstring_output inheritable members */

void
_jive_bitstring_output_init(jive_bitstring_output * self, size_t nbits, struct jive_node * node, size_t index)
{
	self->base.base.class_ = &JIVE_BITSTRING_OUTPUT;
	_jive_value_output_init(&self->base, node, index);
	_jive_bitstring_type_init(&self->type, nbits);
}

const jive_type *
_jive_bitstring_output_get_type(const jive_output * self_)
{
	const jive_bitstring_output * self = (const jive_bitstring_output *) self_;
	return &self->type.base.base;
}

/* bitstring_gate inheritable members */

void
_jive_bitstring_gate_init(jive_bitstring_gate * self, size_t nbits, struct jive_graph * graph, const char name[])
{
	self->base.base.class_ = &JIVE_BITSTRING_GATE;
	_jive_value_gate_init(&self->base, graph, name);
	_jive_bitstring_type_init(&self->type, nbits);
}

const jive_type *
_jive_bitstring_gate_get_type(const jive_gate * self_)
{
	const jive_bitstring_gate * self = (const jive_bitstring_gate *) self_;
	return &self->type.base.base;
}
