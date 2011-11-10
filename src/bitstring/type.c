#include <jive/bitstring/type-private.h>
#include <jive/vsdg/valuetype-private.h>
#include <jive/vsdg/basetype-private.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

#include <stdio.h>
#include <string.h>

const jive_type_class JIVE_BITSTRING_TYPE = {
	.parent = &JIVE_VALUE_TYPE,
	.fini = jive_bitstring_type_fini_, /* override */
	.get_label = jive_bitstring_type_get_label_, /* override */
	.create_input = jive_bitstring_type_create_input_, /* override */
	.create_output = jive_bitstring_type_create_output_, /* override */
	.create_gate = jive_bitstring_type_create_gate_, /* override */
	.equals = jive_bitstring_type_equals_, /* override */
	.copy = jive_bitstring_type_copy_, /* override */
};

const jive_input_class JIVE_BITSTRING_INPUT = {
	.parent = &JIVE_VALUE_INPUT,
	.fini = jive_input_fini_, /* inherit */
	.get_label = jive_input_get_label_, /* inherit */
	.get_type = jive_bitstring_input_get_type_, /* override */
};

const jive_output_class JIVE_BITSTRING_OUTPUT = {
	.parent = &JIVE_VALUE_OUTPUT,
	.fini = jive_output_fini_, /* inherit */
	.get_label = jive_output_get_label_, /* inherit */
	.get_type = jive_bitstring_output_get_type_, /* override */
};

const jive_gate_class JIVE_BITSTRING_GATE = {
	.parent = &JIVE_VALUE_GATE,
	.fini = jive_gate_fini_, /* inherit */
	.get_label = jive_gate_get_label_, /* inherit */
	.get_type = jive_bitstring_gate_get_type_, /* override */
};

/* bitstring_type inheritable members */

void
jive_bitstring_type_fini_( jive_type* self_ )
{
	jive_bitstring_type* self = (jive_bitstring_type*) self_ ;

	jive_value_type_fini_( (jive_type*)&self->base ) ;
}

char *
jive_bitstring_type_get_label_(const jive_type * self_)
{
	const jive_bitstring_type * self = (const jive_bitstring_type *) self_;
	char tmp[16];
	snprintf(tmp, sizeof(tmp), "bits%zd", self->nbits);
	return strdup(tmp);
}

jive_input *
jive_bitstring_type_create_input_(const jive_type * self_, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	const jive_bitstring_type * self = (const jive_bitstring_type *) self_;
	jive_bitstring_input * input = jive_context_malloc(node->graph->context, sizeof(*input));
	input->base.base.class_ = &JIVE_BITSTRING_INPUT;
	jive_bitstring_input_init_(input, self->nbits, node, index, initial_operand);
	return &input->base.base;
}

jive_output *
jive_bitstring_type_create_output_(const jive_type * self_, struct jive_node * node, size_t index)
{
	const jive_bitstring_type * self = (const jive_bitstring_type *) self_;
	jive_bitstring_output * output = jive_context_malloc(node->graph->context, sizeof(*output));
	output->base.base.class_ = &JIVE_BITSTRING_OUTPUT;
	jive_bitstring_output_init_(output, self->nbits, node, index);
	return &output->base.base;
}

jive_gate *
jive_bitstring_type_create_gate_(const jive_type * self_, struct jive_graph * graph, const char * name)
{
	const jive_bitstring_type * self = (const jive_bitstring_type *) self_;
	jive_bitstring_gate * gate = jive_context_malloc(graph->context, sizeof(*gate));
	gate->base.base.class_ = &JIVE_BITSTRING_GATE;
	jive_bitstring_gate_init_(gate, self->nbits, graph, name);
	return &gate->base.base;
}

jive_type *
jive_bitstring_type_copy_(const jive_type * self_, jive_context * context)
{
	const jive_bitstring_type * self = (const jive_bitstring_type *) self_;
	
	jive_bitstring_type * type = jive_context_malloc(context, sizeof(*type));
	
	*type = *self;
	
	return &type->base.base;
}

bool
jive_bitstring_type_equals_(const jive_type * self_, const jive_type * other_)
{
	if (self_->class_ != other_->class_) return false;
	const jive_bitstring_type * self = (const jive_bitstring_type *)self_;
	const jive_bitstring_type * other = (const jive_bitstring_type *)other_;
	
	return self->nbits == other->nbits;
}


static inline void
jive_bitstring_type_init_(jive_bitstring_type * self, size_t nbits)
{
	self->base.base.class_ = &JIVE_BITSTRING_TYPE;
	self->nbits = nbits;
}

/* bitstring_input inheritable members */

void
jive_bitstring_input_init_(jive_bitstring_input * self, size_t nbits, struct jive_node * node, size_t index, jive_output * origin)
{
	jive_value_input_init_(&self->base, node, index, origin);
	jive_bitstring_type_init_(&self->type, nbits);
}

const jive_type *
jive_bitstring_input_get_type_(const jive_input * self_)
{
	const jive_bitstring_input * self = (const jive_bitstring_input *) self_;
	return &self->type.base.base;
}

/* bitstring_output inheritable members */

void
jive_bitstring_output_init_(jive_bitstring_output * self, size_t nbits, struct jive_node * node, size_t index)
{
	self->base.base.class_ = &JIVE_BITSTRING_OUTPUT;
	jive_value_output_init_(&self->base, node, index);
	jive_bitstring_type_init_(&self->type, nbits);
}

const jive_type *
jive_bitstring_output_get_type_(const jive_output * self_)
{
	const jive_bitstring_output * self = (const jive_bitstring_output *) self_;
	return &self->type.base.base;
}

/* bitstring_gate inheritable members */

void
jive_bitstring_gate_init_(jive_bitstring_gate * self, size_t nbits, struct jive_graph * graph, const char name[])
{
	self->base.base.class_ = &JIVE_BITSTRING_GATE;
	jive_value_gate_init_(&self->base, graph, name);
	jive_bitstring_type_init_(&self->type, nbits);
}

const jive_type *
jive_bitstring_gate_get_type_(const jive_gate * self_)
{
	const jive_bitstring_gate * self = (const jive_bitstring_gate *) self_;
	return &self->type.base.base;
}
