#include <jive/arch/stackframe-private.h>
#include <jive/arch/registers.h>
#include <jive/context.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/statetype-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

#include <string.h>

void
_jive_stackframe_fini(jive_stackframe * self)
{
	jive_stackvar_resource * var, * next_var;
	JIVE_LIST_ITERATE_SAFE(self->vars, var, next_var, stackframe_vars_list) {
		var->stackframe = 0;
		JIVE_LIST_REMOVE(self->vars, var, stackframe_vars_list);
	}
}

const jive_stackframe_class JIVE_STACKFRAME_CLASS = {
	.parent = 0,
	.fini = _jive_stackframe_fini
};

void
jive_stackframe_destroy(jive_stackframe * self)
{
	jive_context * context = self->region->graph->context;
	self->class_->fini(self);
	jive_stackslot * slot, * next_slot;
	JIVE_LIST_ITERATE_SAFE(self->slots, slot, next_slot, stackframe_slots_list)
		jive_context_free(context, slot);
	jive_context_free(context, self);
}

jive_stackslot *
jive_stackslot_create(jive_stackframe * stackframe, long offset)
{
	jive_context * context = stackframe->region->graph->context;
	jive_stackslot * self = jive_context_malloc(context, sizeof(*self));
	JIVE_LIST_PUSH_BACK(stackframe->slots, self, stackframe_slots_list);
	self->offset = offset;
	self->stackframe = stackframe;
	return self;
}

/* stackslots */

const jive_type_class JIVE_STACKSLOT_TYPE = {
	.parent = &JIVE_STATE_TYPE,
	.get_label = _jive_stackvar_type_get_label, /* override */
	.create_input = _jive_stackvar_type_create_input, /* override */
	.create_output = _jive_stackvar_type_create_output, /* override */
	.create_resource = _jive_stackvar_type_create_resource, /* override */
	.create_gate = _jive_stackvar_type_create_gate, /* override */
	.equals = _jive_stackvar_type_equals, /* override */
	.accepts = _jive_stackvar_type_accepts /* override */
};

const jive_input_class JIVE_STACKSLOT_INPUT = {
	.parent = &JIVE_INPUT,
	.fini = _jive_input_fini, /* inherit */
	.get_label = _jive_stackvar_input_get_label, /* override */
	.get_type = _jive_stackvar_input_get_type, /* override */
	.get_constraint = _jive_stackvar_input_get_constraint /* override */
};

const jive_output_class JIVE_STACKSLOT_OUTPUT = {
	.parent = &JIVE_OUTPUT,
	.fini = _jive_output_fini, /* inherit */
	.get_label = _jive_stackvar_output_get_label, /* override */
	.get_type = _jive_stackvar_output_get_type, /* override */
	.get_constraint = _jive_stackvar_output_get_constraint /* override */
};

const jive_gate_class JIVE_STACKSLOT_GATE = {
	.parent = &JIVE_GATE,
	.fini = _jive_gate_fini, /* inherit */
	.get_label = _jive_stackvar_gate_get_label, /* override */
	.get_type = _jive_stackvar_gate_get_type, /* override */
	.get_constraint = _jive_gate_get_constraint, /* inherit */
	.create_input = _jive_gate_create_input, /* inherit */
	.create_output = _jive_gate_create_output /* inherit */
};

const jive_resource_class JIVE_STACKSLOT_RESOURCE = {
	.parent = &JIVE_RESOURCE,
	.fini = _jive_stackvar_resource_fini, /* override */
	.get_label = _jive_stackvar_resource_get_label, /* override */
	.get_type = _jive_stackvar_resource_get_type, /* override */
	.can_merge = _jive_stackvar_resource_can_merge, /* override */
	.merge = _jive_stackvar_resource_merge, /* override */
	.get_cpureg = _jive_resource_get_cpureg, /* inherit */
	.get_regcls = _jive_resource_get_regcls, /* inherit */
	.get_real_regcls = _jive_resource_get_real_regcls, /* inherit */
	.add_squeeze = _jive_resource_add_squeeze, /* inherit */
	.sub_squeeze = _jive_resource_sub_squeeze, /* inherit */
	.deny_register = _jive_resource_deny_register, /* inherit */
	.recompute_allowed_registers = _jive_resource_recompute_allowed_registers /* inherit */
};

char *
_jive_stackvar_type_get_label(const jive_type * self_)
{
	const jive_stackvar_type * self = (const jive_stackvar_type *) self_;
	char tmp[80];
	snprintf(tmp, sizeof(tmp), "stack:%s", self->regcls->name);
	return strdup(tmp);
}

jive_input *
_jive_stackvar_type_create_input(const jive_type * self_, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	const jive_stackvar_type * self = (const jive_stackvar_type *) self_;
	jive_stackvar_input * input = jive_context_malloc(node->graph->context, sizeof(*input));
	input->base.base.class_ = &JIVE_STACKSLOT_INPUT;
	_jive_stackvar_input_init(input, self->regcls, node, index, initial_operand);
	return &input->base.base;
}

jive_output *
_jive_stackvar_type_create_output(const jive_type * self_, struct jive_node * node, size_t index)
{
	const jive_stackvar_type * self = (const jive_stackvar_type *) self_;
	jive_stackvar_output * output = jive_context_malloc(node->graph->context, sizeof(*output));
	output->base.base.class_ = &JIVE_STACKSLOT_OUTPUT;
	_jive_stackvar_output_init(output, self->regcls, node, index);
	return &output->base.base;
}

jive_resource *
_jive_stackvar_type_create_resource(const jive_type * self_, struct jive_graph * graph)
{
	const jive_stackvar_type * self = (const jive_stackvar_type *) self_;
	jive_stackvar_resource * resource = jive_context_malloc(graph->context, sizeof(*resource));
	resource->base.base.class_ = &JIVE_STACKSLOT_RESOURCE;
	_jive_stackvar_resource_init(resource, self->regcls, graph);
	return &resource->base.base;
}

jive_gate *
_jive_stackvar_type_create_gate(const jive_type * self_, struct jive_graph * graph, const char * name)
{
	const jive_stackvar_type * self = (const jive_stackvar_type *) self_;
	jive_stackvar_gate * gate = jive_context_malloc(graph->context, sizeof(*gate));
	gate->base.base.class_ = &JIVE_STACKSLOT_GATE;
	_jive_stackvar_gate_init(gate, self->regcls, graph, name);
	return &gate->base.base;
}

bool
_jive_stackvar_type_equals(const jive_type * self_, const jive_type * other_)
{
	const jive_stackvar_type * self = (const jive_stackvar_type *) self_;
	const jive_stackvar_type * other = (const jive_stackvar_type *) other_;
	
	return _jive_type_equals(self_, other_) && (self->regcls == other->regcls);
}

bool
_jive_stackvar_type_accepts(const jive_type * self_, const jive_type * other_)
{
	const jive_stackvar_type * self = (const jive_stackvar_type *) self_;
	const jive_stackvar_type * other = (const jive_stackvar_type *) other_;
	
	return _jive_type_equals(self_, other_) && (self->regcls == other->regcls);
}

void
_jive_stackvar_input_init(jive_stackvar_input * self, const struct jive_regcls * regcls, struct jive_node * node, size_t index, jive_output * origin)
{
	_jive_state_input_init(&self->base, node, index, origin);
	self->type = jive_stackvar_type_create(regcls);
	self->required_slot = 0;
}

char *
_jive_stackvar_input_get_label(const jive_input * self_)
{
	const jive_stackvar_input * self = (const jive_stackvar_input *) self_;
	char tmp[80];
	snprintf(tmp, sizeof(tmp), "stack:%s", self->type.regcls->name);
	return strdup(tmp);
}

const jive_type *
_jive_stackvar_input_get_type(const jive_input * self_)
{
	const jive_stackvar_input * self = (const jive_stackvar_input *) self_;
	return &self->type.base.base;
}

jive_resource *
_jive_stackvar_input_get_constraint(const jive_input * self_)
{
	const jive_stackvar_input * self = (const jive_stackvar_input *) self_;
	jive_stackvar_resource * resource = (jive_stackvar_resource *) _jive_input_get_constraint(self_);
	if (self->required_slot) resource->slot = self->required_slot;
	return &resource->base.base;
}

void
_jive_stackvar_output_init(jive_stackvar_output * self, const struct jive_regcls * regcls, struct jive_node * node, size_t index)
{
	_jive_state_output_init(&self->base, node, index);
	self->type = jive_stackvar_type_create(regcls);
	self->required_slot = 0;
}

char *
_jive_stackvar_output_get_label(const jive_output * self_)
{
	const jive_stackvar_output * self = (const jive_stackvar_output *) self_;
	char tmp[80];
	snprintf(tmp, sizeof(tmp), "stack:%s", self->type.regcls->name);
	return strdup(tmp);
}

const jive_type *
_jive_stackvar_output_get_type(const jive_output * self_)
{
	const jive_stackvar_output * self = (const jive_stackvar_output *) self_;
	return &self->type.base.base;
}

jive_resource *
_jive_stackvar_output_get_constraint(const jive_output * self_)
{
	const jive_stackvar_output * self = (const jive_stackvar_output *) self_;
	jive_stackvar_resource * resource = (jive_stackvar_resource *) _jive_output_get_constraint(self_);
	if (self->required_slot) resource->slot = self->required_slot;
	return &resource->base.base;
}

void
_jive_stackvar_resource_fini(jive_resource * self_)
{
	jive_stackvar_resource * self = (jive_stackvar_resource *) self_;
	if (self->stackframe) JIVE_LIST_REMOVE(self->stackframe->vars, self, stackframe_vars_list);
	_jive_resource_fini(&self->base.base);
}

void
_jive_stackvar_resource_init(jive_stackvar_resource * self, const struct jive_regcls * regcls, struct jive_graph * graph)
{
	_jive_state_resource_init(&self->base, graph);
	self->type = jive_stackvar_type_create(regcls);
	self->stackframe = 0;
	self->slot = 0;
}

char *
_jive_stackvar_resource_get_label(const jive_resource * self_)
{
	const jive_stackvar_resource * self = (const jive_stackvar_resource *) self_;
	char tmp[80];
	snprintf(tmp, sizeof(tmp), "stack:%s", self->type.regcls->name);
	return strdup(tmp);
}

const jive_type *
_jive_stackvar_resource_get_type(const jive_resource * self_)
{
	const jive_stackvar_resource * self = (const jive_stackvar_resource *) self_;
	return &self->type.base.base;
}

bool
_jive_stackvar_resource_can_merge(const jive_resource * self_, const jive_resource * other_)
{
	const jive_stackvar_resource * self = (const jive_stackvar_resource *) self_;
	const jive_stackvar_resource * other = (const jive_stackvar_resource *) other_;
	
	if (!_jive_resource_can_merge(self_, other_)) return false;
	
	if (self->slot && other->slot && (self->slot != other->slot)) return false;
	
	return true;
}

void
_jive_stackvar_resource_merge(jive_resource * self_, jive_resource * other_)
{
	jive_stackvar_resource * self = (jive_stackvar_resource *) self_;
	jive_stackvar_resource * other = (jive_stackvar_resource *) other_;
	
	_jive_resource_merge(self_, other_);
	
	if (other->slot) self->slot = other->slot;
}

char *
_jive_stackvar_gate_get_label(const jive_gate * self_)
{
	const jive_stackvar_gate * self = (const jive_stackvar_gate *) self_;
	char tmp[80];
	snprintf(tmp, sizeof(tmp), "stack:%s", self->type.regcls->name);
	return strdup(tmp);
}

void
_jive_stackvar_gate_init(jive_stackvar_gate * self, const struct jive_regcls * regcls, struct jive_graph * graph, const char * name)
{
	_jive_state_gate_init(&self->base, graph, name);
	self->type = jive_stackvar_type_create(regcls);
}

const jive_type *
_jive_stackvar_gate_get_type(const jive_gate * self_)
{
	const jive_stackvar_gate * self = (const jive_stackvar_gate *) self_;
	return &self->type.base.base;
}

