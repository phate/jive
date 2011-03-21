#include <stdio.h>
#include <string.h>

#include <jive/arch/stackframe-private.h>
#include <jive/arch/registers.h>
#include <jive/context.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/statetype-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

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

const jive_resource_class *
jive_stackframe_get_stackslot_resource_class(jive_stackframe * self, size_t size)
{
	size = (size + 31) & ~31;
	
	jive_stackslot_size_class * cls;
	JIVE_LIST_ITERATE(self->stackslot_size_classes, cls, stackframe_stackslot_size_class_list) {
		if (cls->size == size) return &cls->base;
	}
	
	cls = jive_context_malloc(self->context, sizeof(*cls));
	
	char buffer[64];
	snprintf(buffer, sizeof(buffer), "stackslot%zd", size);
	cls->base.name = jive_context_strdup(self->context, buffer);
	cls->base.limit = 0;
	cls->base.names = 0;
	cls->base.parent = &jive_root_resource_class;
	cls->base.depth = jive_root_resource_class.depth + 1;
	
	cls->stackframe = self;
	cls->size = size;
	JIVE_LIST_PUSH_BACK(self->stackslot_size_classes, cls, stackframe_stackslot_size_class_list);
	
	return &cls->base;
}

const jive_resource_class *
jive_stackframe_get_reserved_stackslot_resource_class(jive_stackframe * self, size_t size, long offset)
{
	const jive_resource_class * parent = jive_stackframe_get_stackslot_resource_class(self, size);
	
	jive_reserved_stackslot_class * cls = jive_context_malloc(self->context, sizeof(*cls));
	
	char buffer[64];
	snprintf(buffer, sizeof(buffer), "stackslot%zd@%ld", size, offset);
	cls->base.base.name = jive_context_strdup(self->context, buffer);
	cls->base.base.limit = 0;
	cls->base.base.names = 0;
	cls->base.base.parent = parent;
	cls->base.base.depth = parent->depth + 1;
	
	cls->base.size = size;
	cls->base.stackframe = self;
	
	jive_stackslot * slot = jive_stackslot_create(&cls->base.base, offset);
	cls->slot = &slot->base;
	cls->base.base.limit = 1;
	cls->base.base.names = &cls->slot;
	
	JIVE_LIST_PUSH_BACK(self->reserved_stackslot_classes, cls, stackframe_reserved_stackslot_class_list);
	
	return &cls->base.base;
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

