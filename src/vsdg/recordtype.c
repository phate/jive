#include <jive/vsdg/recordtype.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/valuetype-private.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

#include <string.h>

/* record_type inheritable members */

static char *
jive_record_type_get_label_(const jive_type * self);
static jive_input *
jive_record_type_create_input_(const jive_type * self, struct jive_node * node,
	size_t index, jive_output * initial_operand);
static jive_output *
jive_record_type_create_output_(const jive_type * self, struct jive_node * node,
	size_t index);
static jive_gate *
jive_record_type_create_gate_(const jive_type * self, struct jive_graph * graph,
	const char name[]);
static bool
jive_record_type_equals_(const jive_type * self, const jive_type * other);
static jive_type *
jive_record_type_copy_(const jive_type * self, struct jive_context * context);

static void
jive_record_input_init_(jive_record_input * self, const jive_record_type * type,
	struct jive_node * node, size_t index, jive_output * origin);
static void
jive_record_input_fini_(jive_input * self);
static const jive_type *
jive_record_input_get_type_(const jive_input * self);

static void
jive_record_output_init_(jive_record_output * self, const jive_record_type * type,
	struct jive_node * node, size_t index);
static void
jive_record_output_fini_(jive_output * self);
static const jive_type *
jive_record_output_get_type_(const jive_output * self);

static void
jive_record_gate_init_(jive_record_gate * self, const jive_record_type * type,
	struct jive_graph * graph, const char name[]);
static void
jive_record_gate_fini_(jive_gate * self);
static const jive_type *
jive_record_gate_get_type_(const jive_gate * self);

const jive_type_class JIVE_RECORD_TYPE = {
	.parent = &JIVE_VALUE_TYPE,
	.fini = jive_value_type_fini_, /* inherit */
	.get_label = jive_record_type_get_label_, /* override */
	.create_input = jive_record_type_create_input_, /* override */
	.create_output = jive_record_type_create_output_, /* override */
	.create_gate = jive_record_type_create_gate_, /* override */
	.equals = jive_record_type_equals_, /* override */
	.copy = jive_record_type_copy_, /* override */
} ;

const jive_input_class JIVE_RECORD_INPUT = { 
	.parent = &JIVE_VALUE_INPUT,
	.fini = jive_record_input_fini_,  /* override */
	.get_label = jive_input_get_label_, /* inherit */
	.get_type = jive_record_input_get_type_, /* override */
} ;

const jive_output_class JIVE_RECORD_OUTPUT = { 
	.parent = &JIVE_VALUE_OUTPUT,
	.fini = jive_record_output_fini_, /* override */
	.get_label = jive_output_get_label_, /* inherit */
	.get_type = jive_record_output_get_type_, /* override */
} ;

const jive_gate_class JIVE_RECORD_GATE = { 
	.parent = &JIVE_VALUE_GATE,
	.fini = jive_record_gate_fini_, /* override */
	.get_label = jive_gate_get_label_, /* inherit */
	.get_type = jive_record_gate_get_type_, /* override */
} ;


void
jive_record_type_init(jive_record_type * self, const jive_record_declaration * decl)
{
	self->base.base.class_ = &JIVE_RECORD_TYPE;
	self->decl = decl;
}

/* record_type inheritable members */

jive_type *
jive_record_type_copy_(const jive_type * self_, jive_context * context)
{
	const jive_record_type * self = (const jive_record_type *) self_;

	jive_record_type * type = jive_context_malloc(context, sizeof(*type));

	jive_record_type_init(type, self->decl);

	return &type->base.base;
}

char *
jive_record_type_get_label_(const jive_type * self_)
{
	return strdup("rcd");
}

jive_input *
jive_record_type_create_input_(const jive_type * self_, struct jive_node * node,
	size_t index, jive_output * initial_operand)
{
	const jive_record_type * self = (const jive_record_type *) self_;
	jive_record_input * input = jive_context_malloc(node->graph->context, sizeof(*input));

	input->base.base.class_ = &JIVE_RECORD_INPUT;
	jive_record_input_init_(input, self, node, index, initial_operand);

	return &input->base.base;
}

jive_output *
jive_record_type_create_output_(const jive_type * self_, struct jive_node * node, size_t index)
{
	const jive_record_type * self = (const jive_record_type *) self_;
	jive_record_output * output = jive_context_malloc(node->graph->context, sizeof(*output));

	output->base.base.class_ = &JIVE_RECORD_OUTPUT;
	jive_record_output_init_(output, self, node, index);

	return &output->base.base;
}

bool
jive_record_type_equals_(const jive_type * self_, const jive_type * other_)
{
	const jive_record_type * self = (const jive_record_type *) self_;
	const jive_record_type * other = (const jive_record_type *) other_;

	return (self->decl == other->decl) ;
}

jive_gate *
jive_record_type_create_gate_(const jive_type * self_, struct jive_graph * graph,
	const char * name)
{
	const jive_record_type * self = (const jive_record_type *) self_;

	jive_record_gate * gate = jive_context_malloc(graph->context, sizeof(*gate));

	gate->base.base.class_ = &JIVE_RECORD_GATE;
	jive_record_gate_init_(gate, self, graph, name);

	return &gate->base.base;
}

/* record_input inheritable members */

void
jive_record_input_init_(jive_record_input * self, const jive_record_type * type,
	struct jive_node * node, size_t index, jive_output * origin)
{
	jive_value_input_init_(&self->base, node, index, origin);
	jive_record_type_init(&self->type, type->decl);
}

void
jive_record_input_fini_(jive_input * self_)
{
	jive_record_input * self = (jive_record_input *) self_;

	self->type.base.base.class_->fini(&self->type.base.base);
	jive_input_fini_(&self->base.base);
}

const jive_type *
jive_record_input_get_type_(const jive_input * self_)
{
	const jive_record_input * self = (const jive_record_input *) self_;
	return &self->type.base.base;
}

/* record_output inheritable members */

void
jive_record_output_init_(jive_record_output * self, const jive_record_type * type,
	struct jive_node * node, size_t index)
{
	jive_value_output_init_(&self->base, node, index);
	jive_record_type_init(&self->type, type->decl);
}

void
jive_record_output_fini_(jive_output * self_)
{
	jive_record_output * self = (jive_record_output *) self_;

	self->type.base.base.class_->fini(&self->type.base.base);
	jive_output_fini_(&self->base.base);
}

const jive_type *
jive_record_output_get_type_(const jive_output * self_)
{
	const jive_record_output * self = (const jive_record_output *) self_;

	return &self->type.base.base;
}

/* record_gate inheritable members */

void
jive_record_gate_init_(jive_record_gate * self, const jive_record_type * type,
	struct jive_graph * graph,  const char name[])
{
	jive_value_gate_init_(&self->base, graph, name);
	jive_record_type_init(&self->type, type->decl);
}

void
jive_record_gate_fini_(jive_gate * self_)
{
	jive_record_gate * self = (jive_record_gate *) self_;

	self->type.base.base.class_->fini(&self->type.base.base);
	jive_gate_fini_(&self->base.base);
}

const jive_type *
jive_record_gate_get_type_(const jive_gate * self_)
{
	const jive_record_gate * self = (const jive_record_gate *) self_;

	return &self->type.base.base;
}
