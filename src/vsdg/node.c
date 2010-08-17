#include <string.h>

#include <jive/internal/compiler.h>

#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/crossings-private.h>
#include <jive/vsdg/cut-private.h>
#include <jive/vsdg/resource-interference-private.h>
#include <jive/vsdg/gate-interference-private.h>
#include <jive/vsdg/regcls-count-private.h>
#include <jive/vsdg/region.h>
#include <jive/arch/registers.h>
#include <jive/util/list.h>

const jive_node_class JIVE_NODE = {
	.parent = 0,
	.fini = _jive_node_fini,
	.get_label = _jive_node_get_label,
	.copy = _jive_node_copy,
	.equiv = _jive_node_equiv,
	.get_aux_regcls = _jive_node_get_aux_regcls
};

void
_jive_node_init(
	jive_node * self,
	struct jive_region * region,
	size_t noperands,
	const struct jive_type * operand_types[const],
	struct jive_output * operands[const],
	size_t noutputs,
	const struct jive_type * output_types[const])
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
	self->shape_location = 0; 
	
	jive_resource_interaction_init(&self->resource_interaction, region->graph->context);
	jive_regcls_count_init(&self->use_count_before);
	jive_regcls_count_init(&self->use_count_after);
	
	self->anchored_regions.first = self->anchored_regions.last = 0;
	
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
	if (self->shape_location)
		jive_node_location_destroy(self->shape_location);
	
	jive_node_unregister_resource_crossings(self);
	
	JIVE_LIST_REMOVE(self->region->nodes, self, region_nodes_list);
	self->region = 0;
	
	while(self->noutputs) jive_output_destroy(self->outputs[self->noutputs - 1]);
	JIVE_LIST_REMOVE(self->graph->bottom, self, graph_bottom_list);
	
	while(self->ninputs) jive_input_destroy(self->inputs[self->ninputs - 1]);
	JIVE_LIST_REMOVE(self->graph->top, self, graph_top_list);
	
	jive_context_free(context, self->inputs);
	jive_context_free(context, self->outputs);
	
	jive_regcls_count_fini(&self->use_count_before, context);
	jive_regcls_count_fini(&self->use_count_after, context);
	jive_resource_interaction_fini(&self->resource_interaction);
	
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
	
jive_node *
_jive_node_copy(const jive_node * self,
	struct jive_region * region,
	struct jive_output * operands[])
{
	jive_node * other = jive_context_malloc(region->graph->context, sizeof(*other));
	const jive_type * operand_types[self->noperands];
	const jive_type * output_types[self->noutputs];
	size_t n;
	for(n=0; n<self->noperands; n++)
		operand_types[n] = jive_input_get_type(self->inputs[n]);
	for(n=0; n<self->noutputs; n++)
		output_types[n] = jive_output_get_type(self->outputs[n]);
	
	other->class_ = self->class_;
	_jive_node_init(other, region,
		self->noperands, operand_types, operands,
		self->noutputs, output_types);
	
	return other;
}

bool
_jive_node_equiv(const jive_node * self, const jive_node * other)
{
	return self->class_ == other->class_;
}

const struct jive_regcls *
_jive_node_get_aux_regcls(const jive_node * self)
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
_jive_node_add_input(jive_node * self, jive_input * input)
{
	if (!self->ninputs) JIVE_LIST_REMOVE(self->graph->top, self, graph_top_list);
	self->ninputs ++;
	self->inputs = jive_context_realloc(self->graph->context, self->inputs, sizeof(jive_input *) * self->ninputs);
	self->inputs[input->index] = input;
	jive_node_invalidate_depth_from_root(self);
	
	if (self->graph->resources_fully_assigned) {
		jive_resource * resource = jive_input_get_constraint(input);
		jive_resource_merge(resource, input->origin->resource);
		jive_resource_assign_input(resource, input);
	}
	
	if (self->region) jive_graph_notify_input_create(self->graph, input);
}

jive_input *
jive_node_add_input(jive_node * self, const jive_type * type, jive_output * initial_operand)
{
	jive_input * input = jive_type_create_input(type, self, self->ninputs, initial_operand);
	_jive_node_add_input(self, input);
	return input;
}

static void
_jive_node_add_output(jive_node * self, jive_output * output)
{
	self->noutputs ++;
	self->outputs = jive_context_realloc(self->graph->context, self->outputs, sizeof(jive_output *) * self->noutputs);
	self->outputs[output->index] = output;
	
	if (self->graph->resources_fully_assigned) {
		jive_resource * resource = jive_output_get_constraint(output);
		jive_resource_assign_output(resource, output);
	}
	if (self->region) jive_graph_notify_output_create(self->graph, output);
}

jive_output *
jive_node_add_output(jive_node * self, const jive_type * type)
{
	jive_output * output = jive_type_create_output(type, self, self->noutputs);
	_jive_node_add_output(self, output);
	return output;
}

jive_input *
jive_node_gate_input(jive_node * self, jive_gate * gate, jive_output * initial_operand)
{
	jive_input * input = jive_gate_create_input(gate, self, self->ninputs, initial_operand);
	_jive_node_add_input(self, input);
	input->gate = gate;
	JIVE_LIST_PUSH_BACK(gate->inputs, input, gate_inputs_list);
	size_t n;
	for(n=0; n<input->index; n++) {
		jive_input * other = self->inputs[n];
		if (!other->gate) continue;
		size_t count = jive_gate_interference_add(gate, other->gate);
		if (count > 0 || !gate->resource || !other->gate->resource) continue;
		jive_resource_interference_add(gate->resource, other->gate->resource);
	}
	return input;
}

jive_output *
jive_node_gate_output(jive_node * self, jive_gate * gate)
{
	jive_output * output = jive_gate_create_output(gate, self, self->noutputs);
	_jive_node_add_output(self, output);
	output->gate = gate;
	JIVE_LIST_PUSH_BACK(gate->outputs, output, gate_outputs_list);
	size_t n;
	for(n=0; n<output->index; n++) {
		jive_output * other = self->outputs[n];
		if (!other->gate) continue;
		size_t count = jive_gate_interference_add(gate, other->gate);
		if (count > 0 || !gate->resource || !other->gate->resource) continue;
		jive_resource_interference_add(gate->resource, other->gate->resource);
	}
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

static void
inc_active_before(jive_node * self, jive_node_resource_interaction * xpoint, size_t count)
{
	jive_resource * resource = xpoint->resource;
	if (xpoint->before_count == 0) {
		jive_resource_interaction_iterator i;
		JIVE_HASH_ITERATE(jive_resource_interaction, self->resource_interaction, i) {
			jive_resource * other = i.entry->resource;
			if (i.entry->before_count == 0) continue;
			if (other == resource) continue;
			jive_resource_interference_add(resource, other);
		}
		
		const jive_regcls * overflow;
		overflow = jive_regcls_count_add(&self->use_count_before, self->graph->context, jive_resource_get_real_regcls(resource));
		(void)overflow;
		DEBUG_ASSERT(overflow == 0);
	}
	xpoint->before_count += count;
}

static void
dec_active_before(jive_node * self, jive_node_resource_interaction * xpoint, size_t count)
{
	xpoint->before_count -= count;
	jive_resource * resource = xpoint->resource;
	if (xpoint->before_count == 0) {
		jive_resource_interaction_iterator i;
		JIVE_HASH_ITERATE(jive_resource_interaction, self->resource_interaction, i) {
			jive_resource * other = i.entry->resource;
			if (i.entry->before_count == 0) continue;
			if (other == resource) continue;
			jive_resource_interference_remove(resource, other);
		}
		jive_regcls_count_sub(&self->use_count_before, self->graph->context, jive_resource_get_real_regcls(resource));
	}
}

static void
inc_active_after(jive_node * self, jive_node_resource_interaction * xpoint, size_t count)
{
	jive_resource * resource = xpoint->resource;
	if (xpoint->after_count == 0) {
		jive_resource_interaction_iterator i;
		JIVE_HASH_ITERATE(jive_resource_interaction, self->resource_interaction, i) {
			jive_resource * other = i.entry->resource;
			if (i.entry->after_count == 0) continue;
			if (other == resource) continue;
			jive_resource_interference_add(resource, other);
		}
		const jive_regcls * overflow;
		overflow = jive_regcls_count_add(&self->use_count_after, self->graph->context, jive_resource_get_real_regcls(resource));
		(void)overflow;
		DEBUG_ASSERT(overflow == 0);
	}
	xpoint->after_count += count;
}

static void
dec_active_after(jive_node * self, jive_node_resource_interaction * xpoint, size_t count)
{
	xpoint->after_count -= count;
	jive_resource * resource = xpoint->resource;
	if (xpoint->after_count == 0) {
		jive_resource_interaction_iterator i;
		JIVE_HASH_ITERATE(jive_resource_interaction, self->resource_interaction, i) {
			jive_resource * other = i.entry->resource;
			if (i.entry->after_count == 0) continue;
			if (other == resource) continue;
			jive_resource_interference_remove(resource, other);
		}
		jive_regcls_count_sub(&self->use_count_after, self->graph->context, jive_resource_get_real_regcls(resource));
	}
}

void
jive_node_add_used_resource(jive_node * self, jive_resource * resource)
{
	jive_node_resource_interaction * xpoint = jive_node_resource_interaction_create(self, resource);
	inc_active_before(self, xpoint, 1);
}

void
jive_node_remove_used_resource(jive_node * self, jive_resource * resource)
{
	jive_node_resource_interaction * xpoint = jive_node_resource_interaction_lookup(self, resource);
	dec_active_before(self, xpoint, 1);
	jive_node_resource_interaction_check_discard(xpoint);
}

void
jive_node_add_defined_resource(jive_node * self, jive_resource * resource)
{
	jive_node_resource_interaction * xpoint = jive_node_resource_interaction_create(self, resource);
	inc_active_after(self, xpoint, 1);
}

void
jive_node_remove_defined_resource(jive_node * self, jive_resource * resource)
{
	jive_node_resource_interaction * xpoint = jive_node_resource_interaction_lookup(self, resource);
	dec_active_after(self, xpoint, 1);
	jive_node_resource_interaction_check_discard(xpoint);
}

void
jive_node_add_crossed_resource(jive_node * self, jive_resource * resource, unsigned int count)
{
	jive_node_resource_interaction * xpoint = jive_node_resource_interaction_create(self, resource);
	
	xpoint->crossed_count += count;
	inc_active_before(self, xpoint, count);
	inc_active_after(self, xpoint, count);
}

void
jive_node_remove_crossed_resource(jive_node * self, jive_resource * resource, unsigned int count)
{
	jive_node_resource_interaction * xpoint = jive_node_resource_interaction_lookup(self, resource);
	
	xpoint->crossed_count -= count;
	dec_active_before(self, xpoint, count);
	dec_active_after(self, xpoint, count);
	
	jive_node_resource_interaction_check_discard(xpoint);
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
jive_node_unregister_resource_crossings(jive_node * self)
{
	size_t n;
	for(n=0; n<self->ninputs; n++)
		jive_input_unregister_resource_crossings(self->inputs[n]);
	for(n=0; n<self->noutputs; n++) {
		jive_input * input;
		JIVE_LIST_ITERATE(self->outputs[n]->users, input, output_users_list)
			jive_input_unregister_resource_crossings(input);
	}
}

void
jive_node_register_resource_crossings(jive_node * self)
{
	size_t n;
	for(n=0; n<self->ninputs; n++)
		jive_input_register_resource_crossings(self->inputs[n]);
	for(n=0; n<self->noutputs; n++) {
		jive_input * input;
		JIVE_LIST_ITERATE(self->outputs[n]->users, input, output_users_list)
			jive_input_register_resource_crossings(input);
	}
}

void
jive_node_remove_all_crossed(jive_node * self)
{
	struct jive_resource_interaction_iterator i = jive_resource_interaction_begin(&self->resource_interaction);
	while(i.entry) {
		jive_node_resource_interaction * xpoint = i.entry;
		jive_resource_interaction_iterator_next(&i);
		if (xpoint->crossed_count)
			jive_node_remove_crossed_resource(self, xpoint->resource, xpoint->crossed_count);
	}
}

void
jive_node_destroy(jive_node * self)
{
	jive_graph_notify_node_destroy(self->graph, self);
	self->class_->fini(self);
	jive_context_free(self->graph->context, self);
}
