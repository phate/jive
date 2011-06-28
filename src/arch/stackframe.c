#include <stdio.h>
#include <string.h>

#include <jive/arch/stackframe-private.h>
#include <jive/arch/registers.h>
#include <jive/context.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/statetype-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

/* FIXME: implement methods of type */
const jive_type_class JIVE_STACKVAR_TYPE = {
	.parent = &JIVE_STATE_TYPE,
	.fini = 0,
	.get_label = 0,
	.create_input = 0,
	.create_output = 0,
	.create_gate = 0,
	.equals = 0,
	.copy = 0
};

void
_jive_stackframe_fini(jive_stackframe * self)
{
	/* FIXME: maybe better to shift deallocation to graph instead?
	stackframes are currently bound to regions, but maybe this
	should migrate into the function anchor node -- this would
	cause problems due to deallocation order */
	jive_stackslot_size_class * cls, * next_cls;
	JIVE_LIST_ITERATE_SAFE(self->stackslot_size_classes, cls, next_cls, stackframe_stackslot_size_class_list) {
		jive_context_free(self->context, (char *)cls->base.name);
		jive_context_free(self->context, cls);
	}
	
	jive_reserved_stackslot_class * res, * next_res;
	JIVE_LIST_ITERATE_SAFE(self->reserved_stackslot_classes, res, next_res, stackframe_reserved_stackslot_class_list) {
		jive_context_free(self->context, (char *)res->base.base.name);
		jive_context_free(self->context, res);
	}
	
	jive_stackslot * slot, * next_slot;
	JIVE_LIST_ITERATE_SAFE(self->slots, slot, next_slot, stackframe_slots_list) {
		jive_context_free(self->context, (char *)slot->base.name);
		jive_context_free(self->context, slot);
	}
}

const jive_stackframe_class JIVE_STACKFRAME_CLASS = {
	.parent = 0,
	.fini = _jive_stackframe_fini
};

void
jive_stackframe_destroy(jive_stackframe * self)
{
	jive_context * context = self->context;
	self->class_->fini(self);
	jive_context_free(context, self);
}

jive_stackslot *
jive_stackslot_create(const jive_resource_class * rescls, long offset)
{
	const jive_stackslot_size_class * cls = (const jive_stackslot_size_class *) rescls;
	jive_stackframe * stackframe = cls->stackframe;
	
	jive_context * context = stackframe->context;
	jive_stackslot * self = jive_context_malloc(context, sizeof(*self));
	
	char buffer[64];
	snprintf(buffer, sizeof(buffer), "stackslot%zd@%ld", cls->size, offset);
	self->base.name = jive_context_strdup(context, buffer);
	self->base.resource_class = rescls;
	self->offset = offset;
	self->stackframe = stackframe;
	JIVE_LIST_PUSH_BACK(stackframe->slots, self, stackframe_slots_list);
	
	return self;
}

#define MAKE_STACKSLOT_CLASS(SIZE) \
const jive_stackslot_size_class jive_stackslot_class_##SIZE##_##SIZE = { \
	.base = { \
		.name = "stack" #SIZE, \
			.limit = 0, .names = NULL, \
			.parent = &jive_root_resource_class, \
			.depth = 1 \
	}, \
	.size = SIZE \
}

MAKE_STACKSLOT_CLASS(8);
MAKE_STACKSLOT_CLASS(16);
MAKE_STACKSLOT_CLASS(32);
MAKE_STACKSLOT_CLASS(64);
MAKE_STACKSLOT_CLASS(128);

const jive_resource_class *
jive_stackframe_get_stackslot_resource_class(jive_stackframe * self, size_t size)
{
	switch(size) {
		case 8: return &jive_stackslot_class_8_8.base;
		case 16: return &jive_stackslot_class_16_16.base;
		case 32: return &jive_stackslot_class_32_32.base;
		case 64: return &jive_stackslot_class_64_64.base;
		case 128: return &jive_stackslot_class_128_128.base;
	}
	
	return 0;
}

#define MAKE_RESERVED_STACKSLOT_CLASS(SIZE, ALIGN, OFFSET) \
const jive_reserved_stackslot_class jive_reserved_stackslot_class_##SIZE##_##OFFSET; \
 \
static const jive_stackslot jive_reserved_stackslot_##SIZE##_##OFFSET = { \
	.base = { \
		.name = "stackslot" #SIZE "@" #OFFSET, \
		.resource_class = &jive_reserved_stackslot_class_##SIZE##_##OFFSET.base.base, \
	}, \
	.offset = OFFSET \
}; \
 \
const jive_reserved_stackslot_class jive_reserved_stackslot_class_##SIZE##_##OFFSET = { \
	.base = { \
		.base = { \
			.name = "stackslot" #SIZE "@" #OFFSET, \
			.limit = 1, \
			.names = (const jive_resource_name *[]) {&jive_reserved_stackslot_##SIZE##_##OFFSET.base}, \
			.parent = &jive_stackslot_class_##SIZE##_##ALIGN.base, \
			.depth = 2, \
		}, \
		.size = SIZE \
	}, \
	.slot = &jive_reserved_stackslot_##SIZE##_##OFFSET.base \
};

MAKE_RESERVED_STACKSLOT_CLASS(32, 32, 0);
MAKE_RESERVED_STACKSLOT_CLASS(32, 32, 4);
MAKE_RESERVED_STACKSLOT_CLASS(32, 32, 8);
MAKE_RESERVED_STACKSLOT_CLASS(32, 32, 12);
MAKE_RESERVED_STACKSLOT_CLASS(32, 32, 16);
MAKE_RESERVED_STACKSLOT_CLASS(32, 32, 20);
MAKE_RESERVED_STACKSLOT_CLASS(32, 32, 24);
MAKE_RESERVED_STACKSLOT_CLASS(32, 32, 28);

const jive_resource_class *
jive_stackframe_get_reserved_stackslot_resource_class(jive_stackframe * self, size_t size, long offset)
{
	switch(size) {
		case 32: switch(offset) {
			case 0: return &jive_reserved_stackslot_class_32_0.base.base;
			case 4: return &jive_reserved_stackslot_class_32_4.base.base;
			case 8: return &jive_reserved_stackslot_class_32_8.base.base;
			case 12: return &jive_reserved_stackslot_class_32_12.base.base;
			case 16: return &jive_reserved_stackslot_class_32_16.base.base;
			case 20: return &jive_reserved_stackslot_class_32_20.base.base;
			case 24: return &jive_reserved_stackslot_class_32_24.base.base;
			case 28: return &jive_reserved_stackslot_class_32_28.base.base;
		};
	};
	
	return 0;
}

/* stackslots */

const jive_type_class JIVE_STACKSLOT_TYPE = {
	.parent = &JIVE_STATE_TYPE,
	.get_label = _jive_stackvar_type_get_label, /* override */
	.create_input = _jive_stackvar_type_create_input, /* override */
	.create_output = _jive_stackvar_type_create_output, /* override */
	.create_gate = _jive_stackvar_type_create_gate, /* override */
	.equals = _jive_stackvar_type_equals, /* override */
};

const jive_input_class JIVE_STACKSLOT_INPUT = {
	.parent = &JIVE_INPUT,
	.fini = _jive_input_fini, /* inherit */
	.get_label = _jive_stackvar_input_get_label, /* override */
	.get_type = _jive_stackvar_input_get_type, /* override */
};

const jive_output_class JIVE_STACKSLOT_OUTPUT = {
	.parent = &JIVE_OUTPUT,
	.fini = _jive_output_fini, /* inherit */
	.get_label = _jive_stackvar_output_get_label, /* override */
	.get_type = _jive_stackvar_output_get_type, /* override */
};

const jive_gate_class JIVE_STACKSLOT_GATE = {
	.parent = &JIVE_GATE,
	.fini = _jive_gate_fini, /* inherit */
	.get_label = _jive_stackvar_gate_get_label, /* override */
	.get_type = _jive_stackvar_gate_get_type, /* override */
};

char *
_jive_stackvar_type_get_label(const jive_type * self_)
{
	const jive_stackvar_type * self = (const jive_stackvar_type *) self_;
	char tmp[80];
	snprintf(tmp, sizeof(tmp), "stackslot%zd", self->size);
	return strdup(tmp);
}

jive_input *
_jive_stackvar_type_create_input(const jive_type * self_, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	const jive_stackvar_type * self = (const jive_stackvar_type *) self_;
	jive_stackvar_input * input = jive_context_malloc(node->graph->context, sizeof(*input));
	input->base.base.class_ = &JIVE_STACKSLOT_INPUT;
	_jive_stackvar_input_init(input, self->size, node, index, initial_operand);
	return &input->base.base;
}

jive_output *
_jive_stackvar_type_create_output(const jive_type * self_, struct jive_node * node, size_t index)
{
	const jive_stackvar_type * self = (const jive_stackvar_type *) self_;
	jive_stackvar_output * output = jive_context_malloc(node->graph->context, sizeof(*output));
	output->base.base.class_ = &JIVE_STACKSLOT_OUTPUT;
	_jive_stackvar_output_init(output, self->size, node, index);
	return &output->base.base;
}

jive_gate *
_jive_stackvar_type_create_gate(const jive_type * self_, struct jive_graph * graph, const char * name)
{
	const jive_stackvar_type * self = (const jive_stackvar_type *) self_;
	jive_stackvar_gate * gate = jive_context_malloc(graph->context, sizeof(*gate));
	gate->base.base.class_ = &JIVE_STACKSLOT_GATE;
	_jive_stackvar_gate_init(gate, self->size, graph, name);
	return &gate->base.base;
}

bool
_jive_stackvar_type_equals(const jive_type * self_, const jive_type * other_)
{
	const jive_stackvar_type * self = (const jive_stackvar_type *) self_;
	const jive_stackvar_type * other = (const jive_stackvar_type *) other_;
	
	return _jive_type_equals(self_, other_) && (self->size == other->size);
}

void
_jive_stackvar_input_init(jive_stackvar_input * self, size_t size, struct jive_node * node, size_t index, jive_output * origin)
{
	_jive_state_input_init(&self->base, node, index, origin);
	self->type = jive_stackvar_type_create(size);
	self->required_slot = 0;
}

char *
_jive_stackvar_input_get_label(const jive_input * self_)
{
	const jive_stackvar_input * self = (const jive_stackvar_input *) self_;
	char tmp[80];
	snprintf(tmp, sizeof(tmp), "stackslot%zd", self->type.size);
	return strdup(tmp);
}

const jive_type *
_jive_stackvar_input_get_type(const jive_input * self_)
{
	const jive_stackvar_input * self = (const jive_stackvar_input *) self_;
	return &self->type.base.base;
}

void
_jive_stackvar_output_init(jive_stackvar_output * self, size_t size, struct jive_node * node, size_t index)
{
	_jive_state_output_init(&self->base, node, index);
	self->type = jive_stackvar_type_create(size);
	self->required_slot = 0;
}

char *
_jive_stackvar_output_get_label(const jive_output * self_)
{
	const jive_stackvar_output * self = (const jive_stackvar_output *) self_;
	char tmp[80];
	snprintf(tmp, sizeof(tmp), "stackslot%zd", self->type.size);
	return strdup(tmp);
}

const jive_type *
_jive_stackvar_output_get_type(const jive_output * self_)
{
	const jive_stackvar_output * self = (const jive_stackvar_output *) self_;
	return &self->type.base.base;
}


char *
_jive_stackvar_gate_get_label(const jive_gate * self_)
{
	const jive_stackvar_gate * self = (const jive_stackvar_gate *) self_;
	char tmp[80];
	snprintf(tmp, sizeof(tmp), "stackslot%zd", self->type.size);
	return strdup(tmp);
}

void
_jive_stackvar_gate_init(jive_stackvar_gate * self, size_t size, struct jive_graph * graph, const char * name)
{
	_jive_state_gate_init(&self->base, graph, name);
	self->type = jive_stackvar_type_create(size);
}

const jive_type *
_jive_stackvar_gate_get_type(const jive_gate * self_)
{
	const jive_stackvar_gate * self = (const jive_stackvar_gate *) self_;
	return &self->type.base.base;
}

