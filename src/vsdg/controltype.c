#include <jive/vsdg/controltype.h>
#include <jive/vsdg/controltype-private.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>
#include <jive/util/list.h>
#include <jive/debug-private.h>

#include <jive/vsdg/node.h>
#include <jive/vsdg/graph.h>

static const jive_control_type jive_control_type_singleton = {
	.base = { .class_ = &JIVE_CONTROL_TYPE }
};

const jive_type_class JIVE_CONTROL_TYPE = {
	.parent = &JIVE_TYPE,
	.get_label = _jive_type_get_label, /* inherit */
	.create_input = _jive_control_type_create_input, /* override */
	.create_output = _jive_control_type_create_output, /* override */
	.create_resource = _jive_control_type_create_resource, /* override */
	.create_gate = _jive_type_create_gate, /* inherit */
	.equals = _jive_type_equals, /* inherit */
	.accepts = _jive_type_accepts /* inherit */
};

const jive_input_class JIVE_CONTROL_INPUT = {
	.parent = &JIVE_INPUT,
	.fini = _jive_control_input_fini, /* override */
	.get_label = _jive_input_get_label, /* inherit */
	.get_type = _jive_control_input_get_type, /* override */
	.get_constraint = _jive_input_get_constraint /* inherit */
};

const jive_output_class JIVE_CONTROL_OUTPUT = {
	.parent = &JIVE_OUTPUT,
	.fini = _jive_output_fini, /* inherit */
	.get_label = _jive_output_get_label, /* inherit */
	.get_type = _jive_control_output_get_type, /* override */
	.get_constraint = _jive_output_get_constraint /* inherit */
};

const jive_resource_class JIVE_CONTROL_RESOURCE = {
	.parent = &JIVE_RESOURCE,
	.fini = _jive_resource_fini, /* inherit */
	.get_label = _jive_resource_get_label, /* inherit */
	.get_type = _jive_control_resource_get_type, /* override */
	.can_merge = _jive_resource_can_merge, /* inherit */
	.merge = _jive_resource_merge, /* inherit */
	.get_cpureg = _jive_resource_get_cpureg, /* inherit */
	.get_regcls = _jive_resource_get_regcls, /* inherit */
	.get_real_regcls = _jive_resource_get_real_regcls, /* inherit */
	.add_squeeze = _jive_resource_add_squeeze, /* inherit */
	.sub_squeeze = _jive_resource_sub_squeeze, /* inherit */
	.deny_register = _jive_resource_deny_register, /* inherit */
	.recompute_allowed_registers = _jive_resource_recompute_allowed_registers /* inherit */
};

jive_input *
_jive_control_type_create_input(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	jive_control_input * input = jive_context_malloc(node->graph->context, sizeof(*input));
	input->base.class_ = &JIVE_CONTROL_INPUT;
	_jive_control_input_init(input, node, index, initial_operand);
	return &input->base; 
}

jive_output *
_jive_control_type_create_output(const jive_type * self, struct jive_node * node, size_t index)
{
	jive_control_output * output = jive_context_malloc(node->graph->context, sizeof(*output));
	output->base.class_ = &JIVE_CONTROL_OUTPUT;
	_jive_control_output_init(output, node, index);
	return &output->base; 
}

jive_resource *
_jive_control_type_create_resource(const jive_type * self, struct jive_graph * graph)
{
	jive_control_resource * resource = jive_context_malloc(graph->context, sizeof(*resource));
	resource->base.class_ = &JIVE_CONTROL_RESOURCE;
	_jive_control_resource_init(resource, graph);
	return &resource->base; 
}

void
_jive_control_input_init(jive_control_input * self, struct jive_node * node, size_t index, jive_output * origin)
{
	_jive_input_init(&self->base, node, index, origin);
	DEBUG_ASSERT(origin->node->region->anchor_node == 0);
	origin->node->region->anchor_node = node;
	JIVE_LIST_PUSH_BACK(node->anchored_regions, origin->node->region, node_anchored_regions_list);
}

void
_jive_control_input_fini(jive_input * self_)
{
	jive_control_input * self = (jive_control_input *)self_;
	if (self->base.origin->node->region->anchor_node == self->base.node) {
		JIVE_LIST_REMOVE(self->base.node->anchored_regions, self->base.origin->node->region, node_anchored_regions_list);
		self->base.origin->node->region->anchor_node = 0;
	}
	_jive_input_fini(&self->base);
}

const jive_type *
_jive_control_input_get_type(const jive_input * self)
{
	return &jive_control_type_singleton.base;
}

void
_jive_control_output_init(jive_control_output * self, struct jive_node * node, size_t index)
{
	_jive_output_init(&self->base, node, index);
}

const jive_type *
_jive_control_output_get_type(const jive_output * self)
{
	return &jive_control_type_singleton.base;
}

void
_jive_control_resource_init(jive_control_resource * self, struct jive_graph * graph)
{
	_jive_resource_init(&self->base, graph);
}

const jive_type *
_jive_control_resource_get_type(const jive_resource * self)
{
	return &jive_control_type_singleton.base;
}
