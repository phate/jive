#include <jive/vsdg/recordtype.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/valuetype-private.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

#include <string.h>

/* record_type inheritable members */

static char *
_jive_record_type_get_label(const jive_type * self);
static jive_input *
_jive_record_type_create_input(const jive_type * self, struct jive_node * node,
	size_t index, jive_output * initial_operand);
static jive_output *
_jive_record_type_create_output(const jive_type * self, struct jive_node * node,
	size_t index);
static jive_gate *
_jive_record_type_create_gate(const jive_type * self, struct jive_graph * graph,
	const char name[]);
static bool
_jive_record_type_equals(const jive_type * self, const jive_type * other);
static jive_type *
_jive_record_type_copy(const jive_type * self, struct jive_context * context);

static void
_jive_record_input_init(jive_record_input * self, const jive_record_type * type,
	struct jive_node * node, size_t index, jive_output * origin);
static void
_jive_record_input_fini(jive_input * self);
static const jive_type *
_jive_record_input_get_type(const jive_input * self);

static void
_jive_record_output_init(jive_record_output * self, const jive_record_type * type,
	struct jive_node * node, size_t index);
static void
_jive_record_output_fini(jive_output * self);
static const jive_type *
_jive_record_output_get_type(const jive_output * self);

static void
_jive_record_gate_init(jive_record_gate * self, const jive_record_type * type,
	struct jive_graph * graph, const char name[]);
static void
_jive_record_gate_fini(jive_gate * self);
static const jive_type *
_jive_record_gate_get_type(const jive_gate * self);

const jive_type_class JIVE_RECORD_TYPE = {
	.parent = &JIVE_VALUE_TYPE,
	.fini = _jive_value_type_fini, /* inherit */
	.get_label = _jive_record_type_get_label, /* override */
	.create_input = _jive_record_type_create_input, /* override */
	.create_output = _jive_record_type_create_output, /* override */
	.create_gate = _jive_record_type_create_gate, /* override */
	.equals = _jive_record_type_equals, /* override */
	.copy = _jive_record_type_copy, /* override */
} ;

const jive_input_class JIVE_RECORD_INPUT = { 
	.parent = &JIVE_VALUE_INPUT,
	.fini = _jive_record_input_fini,  /* override */
	.get_label = jive_input_get_label_, /* inherit */
	.get_type = _jive_record_input_get_type, /* override */
} ;

const jive_output_class JIVE_RECORD_OUTPUT = { 
	.parent = &JIVE_VALUE_OUTPUT,
	.fini = _jive_record_output_fini, /* override */
	.get_label = jive_output_get_label_, /* inherit */
	.get_type = _jive_record_output_get_type, /* override */
} ;

const jive_gate_class JIVE_RECORD_GATE = { 
	.parent = &JIVE_VALUE_GATE,
	.fini = _jive_record_gate_fini, /* override */
	.get_label = jive_gate_get_label_, /* inherit */
	.get_type = _jive_record_gate_get_type, /* override */
} ;


void
jive_record_type_init(jive_record_type * self, const jive_record_layout * layout)
{
	self->base.base.class_ = &JIVE_RECORD_TYPE;
	self->layout = layout ; 
}

/* record_type inheritable members */

jive_type *
_jive_record_type_copy(const jive_type * self_, jive_context * context)
{
	const jive_record_type * self = (const jive_record_type *) self_;

	jive_record_type * type = jive_context_malloc(context, sizeof(*type));

	jive_record_type_init(type, self->layout);

	return &type->base.base;
}

char *
_jive_record_type_get_label(const jive_type * self_)
{
	return strdup("rcd");
}

jive_input *
_jive_record_type_create_input(const jive_type * self_, struct jive_node * node,
	size_t index, jive_output * initial_operand)
{
	const jive_record_type * self = (const jive_record_type *) self_;
	jive_record_input * input = jive_context_malloc(node->graph->context, sizeof(*input));

	input->base.base.class_ = &JIVE_RECORD_INPUT;
	_jive_record_input_init(input, self, node, index, initial_operand);

	return &input->base.base;
}

jive_output *
_jive_record_type_create_output(const jive_type * self_, struct jive_node * node, size_t index)
{
	const jive_record_type * self = (const jive_record_type *) self_;
	jive_record_output * output = jive_context_malloc(node->graph->context, sizeof(*output));

	output->base.base.class_ = &JIVE_RECORD_OUTPUT;
	_jive_record_output_init(output, self, node, index);

	return &output->base.base;
}

bool
_jive_record_type_equals(const jive_type * self_, const jive_type * other_)
{
	const jive_record_type * self = (const jive_record_type *) self_;
	const jive_record_type * other = (const jive_record_type *) other_;

	return (self->layout == other->layout) ;
}

jive_gate *
_jive_record_type_create_gate(const jive_type * self_, struct jive_graph * graph,
	const char * name)
{
	const jive_record_type * self = (const jive_record_type *) self_;

	jive_record_gate * gate = jive_context_malloc(graph->context, sizeof(*gate));

	gate->base.base.class_ = &JIVE_RECORD_GATE;
	_jive_record_gate_init(gate, self, graph, name);

	return &gate->base.base;
}

/* record_input inheritable members */

void
_jive_record_input_init(jive_record_input * self, const jive_record_type * type,
	struct jive_node * node, size_t index, jive_output * origin)
{
	_jive_value_input_init(&self->base, node, index, origin);	
	jive_record_type_init(&self->type, type->layout);
}

void
_jive_record_input_fini(jive_input * self_)
{
	jive_record_input * self = (jive_record_input *) self_;

	self->type.base.base.class_->fini(&self->type.base.base);
	jive_input_fini_(&self->base.base);
}

const jive_type *
_jive_record_input_get_type(const jive_input * self_)
{
	const jive_record_input * self = (const jive_record_input *) self_;
	return &self->type.base.base;
}

/* record_output inheritable members */

void
_jive_record_output_init(jive_record_output * self, const jive_record_type * type,
	struct jive_node * node, size_t index)
{
	_jive_value_output_init(&self->base, node, index);
	jive_record_type_init(&self->type, type->layout);
}

void
_jive_record_output_fini(jive_output * self_)
{
	jive_record_output * self = (jive_record_output *) self_;

	self->type.base.base.class_->fini(&self->type.base.base);
	jive_output_fini_(&self->base.base);
}

const jive_type *
_jive_record_output_get_type(const jive_output * self_)
{
	const jive_record_output * self = (const jive_record_output *) self_;

	return &self->type.base.base;
}

/* record_gate inheritable members */

void
_jive_record_gate_init(jive_record_gate * self, const jive_record_type * type,
	struct jive_graph * graph,  const char name[])
{
	_jive_value_gate_init(&self->base, graph, name);
	jive_record_type_init(&self->type, type->layout);
}

void
_jive_record_gate_fini(jive_gate * self_)
{
	jive_record_gate * self = (jive_record_gate *) self_;

	self->type.base.base.class_->fini(&self->type.base.base);
	jive_gate_fini_(&self->base.base);
}

const jive_type *
_jive_record_gate_get_type(const jive_gate * self_)
{
	const jive_record_gate * self = (const jive_record_gate *) self_;

	return &self->type.base.base;
}
