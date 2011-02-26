#include <jive/bitstring/type-private.h>
#include <jive/vsdg/valuetype-private.h>
#include <jive/vsdg/basetype-private.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

#include <stdio.h>
#include <string.h>

const jive_type_class JIVE_BITSTRING_TYPE = {
	.parent = &JIVE_VALUE_TYPE,
	.get_label = _jive_bitstring_type_get_label, /* override */
	.create_input = _jive_bitstring_type_create_input, /* override */
	.create_output = _jive_bitstring_type_create_output, /* override */
	.create_resource = _jive_bitstring_type_create_resource, /* override */
	.create_gate = _jive_bitstring_type_create_gate, /* override */
	.equals = _jive_type_equals, /* inherit */
	.accepts = _jive_type_accepts /* inherit */
};

const jive_input_class JIVE_BITSTRING_INPUT = {
	.parent = &JIVE_VALUE_INPUT,
	.fini = _jive_input_fini, /* inherit */
	.get_label = _jive_input_get_label, /* inherit */
	.get_type = _jive_bitstring_input_get_type, /* override */
	.get_constraint = _jive_value_input_get_constraint /* inherit */
};

const jive_output_class JIVE_BITSTRING_OUTPUT = {
	.parent = &JIVE_VALUE_OUTPUT,
	.fini = _jive_output_fini, /* inherit */
	.get_label = _jive_output_get_label, /* inherit */
	.get_type = _jive_bitstring_output_get_type, /* override */
	.get_constraint = _jive_value_output_get_constraint /* inherit */
};

const jive_gate_class JIVE_BITSTRING_GATE = {
	.parent = &JIVE_VALUE_GATE,
	.fini = _jive_gate_fini, /* inherit */
	.get_label = _jive_gate_get_label, /* inherit */
	.get_type = _jive_bitstring_gate_get_type, /* override */
	.get_constraint = _jive_value_gate_get_constraint, /* inherit */
	.create_input = _jive_value_gate_create_input, /* override */
	.create_output = _jive_value_gate_create_output /* override */
};

const jive_resource_class JIVE_BITSTRING_RESOURCE = {
	.parent = &JIVE_VALUE_RESOURCE,
	.fini = _jive_value_resource_fini, /* inherit */
	.get_label = _jive_resource_get_label, /* inherit */
	.get_type = _jive_bitstring_resource_get_type, /* override */
	.can_merge = _jive_bitstring_resource_can_merge, /* override */
	.merge = _jive_value_resource_merge, /* inherit */
	.get_cpureg = _jive_value_resource_get_cpureg, /* inherit */
	.get_regcls = _jive_value_resource_get_regcls, /* inherit */
	.get_real_regcls = _jive_value_resource_get_real_regcls, /* inherit */
	.add_squeeze = _jive_value_resource_add_squeeze, /* inherit */
	.sub_squeeze = _jive_value_resource_sub_squeeze, /* inherit */
	.deny_register = _jive_value_resource_deny_register, /* inherit */
	.recompute_allowed_registers = _jive_value_resource_recompute_allowed_registers /* inherit */
};

/* bitstring_type inheritable members */

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

jive_resource *
_jive_bitstring_type_create_resource(const jive_type * self_, struct jive_graph * graph)
{
	const jive_bitstring_type * self = (const jive_bitstring_type *) self_;
	jive_bitstring_resource * resource = jive_context_malloc(graph->context, sizeof(*resource));
	resource->base.base.class_ = &JIVE_BITSTRING_RESOURCE;
	_jive_bitstring_resource_init(resource, self->nbits, graph);
	return &resource->base.base;
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

/* bitstring_resource inheritable members */

void
_jive_bitstring_resource_init(jive_bitstring_resource * self, size_t nbits, struct jive_graph * graph)
{
	self->base.base.class_ = &JIVE_BITSTRING_RESOURCE;
	_jive_value_resource_init(&self->base, graph);
	_jive_bitstring_type_init(&self->type, nbits);
}

const jive_type *
_jive_bitstring_resource_get_type(const jive_resource * self_)
{
	const jive_bitstring_resource * self = (const jive_bitstring_resource *) self_;
	return &self->type.base.base;
}

bool
_jive_bitstring_resource_can_merge(const jive_resource * self_, const jive_resource * other_)
{
	const jive_bitstring_resource * self = (const jive_bitstring_resource *) self_;
	const jive_bitstring_resource * other = (const jive_bitstring_resource *) other_;
	
	/* not sure this check should be done at all... merging resources of
	mismatching types is a logic error that should probably throw
	a fatal exception error instead of silently denying merge attempts;
	removing the check would allow this member function to be inherited
	instead of overridden, which generally seems a good idea */
	
	return (self->type.nbits == other->type.nbits) && _jive_value_resource_can_merge(self_, other_);
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
