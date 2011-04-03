#include <jive/vsdg/node-private.h>

#include <string.h>

#include <jive/internal/compiler.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/gate-interference-private.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/resource-private.h>
#include <jive/vsdg/variable.h>
#include <jive/util/list.h>

#include "debug.h"

const jive_node_class JIVE_NODE = {
	.parent = 0,
	.name = "NODE",
	.fini = _jive_node_fini,
	.get_default_normal_form = _jive_node_get_default_normal_form,
	.get_label = _jive_node_get_label,
	.get_attrs = _jive_node_get_attrs,
	.match_attrs = _jive_node_match_attrs,
	.create = _jive_node_create,
	.get_aux_rescls = _jive_node_get_aux_rescls
};

void
_jive_node_init(
	jive_node * self,
	struct jive_region * region,
	size_t noperands,
	const struct jive_type * const operand_types[],
	struct jive_output * const operands[],
	size_t noutputs,
	const struct jive_type * const output_types[])
{
	self->graph = region->graph;
	self->depth_from_root = 0;
	self->nsuccessors = 0;
	
	self->ninputs = 0;
	self->inputs = 0;
	
	self->noutputs = 0;
	self->outputs = 0;
	
	self->reserved = 0;
	
	/* set region to zero for now to inhibit notification about
	created inputs/outputs while constructing the node */
	self->region = 0;
	
	JIVE_LIST_PUSH_BACK(self->graph->top, self, graph_top_list);
	JIVE_LIST_PUSH_BACK(self->graph->bottom, self, graph_bottom_list);
	
	size_t n;
	for(n=0; n<noperands; n++)
		jive_node_add_input(self, operand_types[n], operands[n]);
	self->noperands = self->ninputs;
	
	for(n=0; n<noutputs; n++)
		jive_node_add_output(self, output_types[n]);
	
	self->ntraverser_slots = 0;
	self->traverser_slots = 0;
	
	JIVE_LIST_PUSH_BACK(region->nodes, self, region_nodes_list);
	self->region = region;
	
	jive_graph_notify_node_create(self->graph, self);
}

void
_jive_node_fini(jive_node * self)
{
	jive_context * context = self->graph->context;
	DEBUG_ASSERT(self->region);
	
	JIVE_LIST_REMOVE(self->region->nodes, self, region_nodes_list);
	
	while(self->noutputs) jive_output_destroy(self->outputs[self->noutputs - 1]);
	
	while(self->ninputs) jive_input_destroy(self->inputs[self->ninputs - 1]);
	
	JIVE_LIST_REMOVE(self->graph->bottom, self, graph_bottom_list);
	JIVE_LIST_REMOVE(self->graph->top, self, graph_top_list);
	if (self == self->region->top) self->region->top = NULL;
	if (self == self->region->bottom) self->region->bottom = NULL;
	
	self->region = 0;
	jive_context_free(context, self->inputs);
	jive_context_free(context, self->outputs);
	
	if (self->traverser_slots) {
		size_t n;
		for(n=0; n<self->ntraverser_slots; n++)
			jive_context_free(context, self->traverser_slots[n]);
		jive_context_free(context, self->traverser_slots);
	}
}

char *
_jive_node_get_label(const jive_node * self)
{
	return strdup("NODE");
}

bool
_jive_node_match_attrs(const jive_node * self, const jive_node_attrs * other)
{
	return true;
}

const jive_node_attrs *
_jive_node_get_attrs(const jive_node * self)
{
	return 0;
}

jive_node *
_jive_node_create(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[])
{
	jive_node * other = jive_context_malloc(region->graph->context, sizeof(*other));
	const jive_type * operand_types[noperands];
	size_t n;
	for(n=0; n<noperands; n++)
		operand_types[n] = jive_output_get_type(operands[n]);
	
	other->class_ = &JIVE_NODE;
	_jive_node_init(other, region,
		noperands, operand_types, operands,
		0, 0);
	
	return other;
}

const struct jive_node_normal_form *
_jive_node_get_default_normal_form(const jive_node * self)
{
	return 0;
}

const struct jive_resource_class *
_jive_node_get_aux_rescls(const jive_node * self)
{
	return 0;
}

jive_node *
jive_node_create(
	struct jive_region * region,
	size_t noperands,
	const struct jive_type * operand_types[const],
	struct jive_output * operands[const],
	size_t noutputs,
	const struct jive_type * output_types[const])
{
	jive_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->class_ = &JIVE_NODE;
	_jive_node_init(node, region, noperands, operand_types, operands, noutputs, output_types);
	
	return node;
}

static void
jive_node_add_input_(jive_node * self, jive_input * input)
{
	DEBUG_ASSERT(!self->graph->resources_fully_assigned);
	
	if (!self->ninputs) JIVE_LIST_REMOVE(self->graph->top, self, graph_top_list);
	self->ninputs ++;
	self->inputs = jive_context_realloc(self->graph->context, self->inputs, sizeof(jive_input *) * self->ninputs);
	self->inputs[input->index] = input;
	jive_node_invalidate_depth_from_root(self);
	
	if (self->region) jive_graph_notify_input_create(self->graph, input);
}

jive_input *
jive_node_add_input(jive_node * self, const jive_type * type, jive_output * initial_operand)
{
	jive_input * input = jive_type_create_input(type, self, self->ninputs, initial_operand);
	jive_node_add_input_(self, input);
	return input;
}

static void
jive_node_add_output_(jive_node * self, jive_output * output)
{
	DEBUG_ASSERT(!self->graph->resources_fully_assigned);
	
	self->noutputs ++;
	self->outputs = jive_context_realloc(self->graph->context, self->outputs, sizeof(jive_output *) * self->noutputs);
	self->outputs[output->index] = output;
	
	if (self->region) jive_graph_notify_output_create(self->graph, output);
}

jive_output *
jive_node_add_output(jive_node * self, const jive_type * type)
{
	jive_output * output = jive_type_create_output(type, self, self->noutputs);
	jive_node_add_output_(self, output);
	return output;
}

jive_input *
jive_node_gate_input(jive_node * self, jive_gate * gate, jive_output * initial_operand)
{
	jive_input * input = jive_gate_create_input(gate, self, self->ninputs, initial_operand);
	input->gate = gate;
	JIVE_LIST_PUSH_BACK(gate->inputs, input, gate_inputs_list);
	size_t n;
	for(n=0; n<input->index; n++) {
		jive_input * other = self->inputs[n];
		if (!other->gate) continue;
		jive_gate_interference_add(self->graph, gate, other->gate);
	}
	jive_node_add_input_(self, input);
	return input;
}

jive_output *
jive_node_gate_output(jive_node * self, jive_gate * gate)
{
	jive_output * output = jive_gate_create_output(gate, self, self->noutputs);
	output->gate = gate;
	JIVE_LIST_PUSH_BACK(gate->outputs, output, gate_outputs_list);
	size_t n;
	for(n=0; n<output->index; n++) {
		jive_output * other = self->outputs[n];
		if (!other->gate) continue;
		jive_gate_interference_add(self->graph, gate, other->gate);
	}
	jive_node_add_output_(self, output);
	return output;
}

void
jive_node_add_successor(jive_node * self)
{
	if (unlikely(self->nsuccessors == 0))
		JIVE_LIST_REMOVE(self->graph->bottom, self, graph_bottom_list);
	
	self->nsuccessors ++;
}

void
jive_node_remove_successor(jive_node * self)
{
	self->nsuccessors --;
	if (unlikely(self->nsuccessors == 0))
		JIVE_LIST_PUSH_BACK(self->graph->bottom, self, graph_bottom_list);
}

void
jive_node_invalidate_depth_from_root(jive_node * self)
{
	size_t new_depth_from_root = 0, n;
	for(n=0; n<self->ninputs; n++)
		if (self->inputs[n]->origin->node->depth_from_root + 1 > new_depth_from_root)
			 new_depth_from_root = self->inputs[n]->origin->node->depth_from_root + 1;
	
	if (self->depth_from_root == new_depth_from_root) return;
	self->depth_from_root = new_depth_from_root;
	
	for(n=0; n<self->noutputs; n++) {
		jive_input * user = self->outputs[n]->users.first;
		while(user) {
			jive_node_invalidate_depth_from_root(user->node);
			user = user->output_users_list.next;
		}
	}
}

void
jive_node_auto_merge_variables(jive_node * self)
{
	size_t n;
	for(n = 0; n < self->ninputs; n++)
		jive_input_auto_merge_variable(self->inputs[n]);
	for(n = 0; n < self->noutputs; n++)
		jive_output_auto_merge_variable(self->outputs[n]);
}

void
jive_node_get_use_count_input(const jive_node * self, jive_resource_class_count * use_count, jive_context * context)
{
	jive_resource_class_count_clear(use_count, context);
	
	size_t n;
	for(n = 0; n<self->ninputs; n++) {
		jive_input * input = self->inputs[n];
		
		/* filter out multiple inputs using the same value
		FIXME: this assumes that all inputs have the same resource
		class requirement! */
		if (!input->gate) {
			bool duplicate = false;
			size_t k;
			for(k = 0; k<n; k++) {
				if (self->inputs[k]->origin == input->origin)
					duplicate = true;
			}
			if (duplicate) continue;
		}
		
		const jive_resource_class * rescls;
		if (input->ssavar) rescls = input->ssavar->variable->rescls;
		else if (input->gate) rescls = input->gate->required_rescls;
		else rescls = input->required_rescls;
		
		jive_resource_class_count_add(use_count, context, rescls);
	}
}

void
jive_node_get_use_count_output(const jive_node * self, jive_resource_class_count * use_count, jive_context * context)
{
	jive_resource_class_count_clear(use_count, context);
	
	size_t n;
	for(n = 0; n<self->noutputs; n++) {
		jive_output * output = self->outputs[n];
		
		const jive_resource_class * rescls;
		if (output->ssavar) rescls = output->ssavar->variable->rescls;
		else if (output->gate) rescls = output->gate->required_rescls;
		else rescls = output->required_rescls;
		
		jive_resource_class_count_add(use_count, context, rescls);
	}
}


void
jive_node_destroy(jive_node * self)
{
	jive_graph_notify_node_destroy(self->graph, self);
	self->class_->fini(self);
	jive_context_free(self->graph->context, self);
}
