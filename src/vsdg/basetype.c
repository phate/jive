#include <string.h>

#include <jive/debug-private.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/crossings-private.h>
#include <jive/vsdg/resource-interference-private.h>
#include <jive/vsdg/gate-interference-private.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/cut.h>
#include <jive/util/list.h>

/* static type instance, to be returned by type queries */
const jive_type jive_type_singleton = {
	.class_ = &JIVE_TYPE
};


static inline void
crossing_range_through_apply(jive_region * passed_region, void (*function)(void * closure, jive_node * node), void * closure)
{
	jive_cut * cut;
	JIVE_LIST_ITERATE(passed_region->cuts, cut, region_cuts_list) {
		jive_node_location * loc;
		JIVE_LIST_ITERATE(cut->nodes, loc, cut_nodes_list) {
			jive_node * node = loc->node;
			
			jive_region * region;
			JIVE_LIST_ITERATE(node->anchored_regions, region, node_anchored_regions_list)
				crossing_range_through_apply(region, function, closure);
			
			function(closure, node);
		}
	}
}

static inline void
crossing_range_towards_apply(jive_node * current_node, jive_region * current_region, jive_node * target_node,
	void (*function)(void * closure, jive_node * node), void * closure)
{
	if (current_region != target_node->region) {
		crossing_range_towards_apply(current_node, current_region, target_node->region->anchor_node, function, closure);
		current_node = 0;
	}
	
	jive_node_location * current;
	if (!current_node) current = jive_region_begin(target_node->region);
	else current = jive_node_location_next_in_region(current_node->shape_location);
	
	for(;;) {
		jive_node * node = current->node;
		if (node == target_node) break;
		
		jive_region * region;
		JIVE_LIST_ITERATE(node->anchored_regions, region, node_anchored_regions_list)
			crossing_range_through_apply(region, function, closure);
		
		function(closure, node);
		current = jive_node_location_next_in_region(current);
	}
}

static inline void
jive_input_crossing_range_apply(jive_input * self, void (*function)(void * closure, jive_node * node), void * closure)
{
	/* FIXME: gross hack to ignore "Control" edges,
	should be resolved using method override instead */
	if (self->class_ == &JIVE_CONTROL_INPUT) return;
	
	if (!self->node->shape_location) return;
	if (!self->node->region) return; /* destroying node, range not applicable anymore */
	
	jive_node * begin;
	if (self->origin->node->shape_location) begin = self->origin->node;
	else if (self->resource->hovering_region) begin = 0;
	else return;
	
	crossing_range_towards_apply(begin, self->origin->node->region, self->node, function, closure);
}

char *
_jive_type_get_label(const jive_type * self)
{
	return strdup("X");
}

jive_input *
_jive_type_create_input(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	jive_input * input = jive_context_malloc(node->graph->context, sizeof(*input));
	input->class_ = &JIVE_INPUT;
	_jive_input_init(input, node, index, initial_operand);
	return input;
}

jive_output *
_jive_type_create_output(const jive_type * self, struct jive_node * node, size_t index)
{
	jive_output * output = jive_context_malloc(node->graph->context, sizeof(*output));
	output->class_ = &JIVE_OUTPUT;
	_jive_output_init(output, node, index);
	return output;
}

jive_resource *
_jive_type_create_resource(const jive_type * self, struct jive_graph * graph)
{
	jive_resource * resource = jive_context_malloc(graph->context, sizeof(*resource));
	resource->class_ = &JIVE_RESOURCE;
	_jive_resource_init(resource, graph);
	return resource;
}

jive_gate *
_jive_type_create_gate(const jive_type * self, struct jive_graph * graph, const char * name)
{
	jive_gate * gate = jive_context_malloc(graph->context, sizeof(*gate));
	gate->class_ = &JIVE_GATE;
	_jive_gate_init(gate, graph, name);
	return gate;
}

bool
_jive_type_equals(const jive_type * self, const jive_type * other)
{
	return self->class_ == other->class_;
}

bool
_jive_type_accepts(const jive_type * self, const jive_type * other)
{
	return self->class_ == other->class_;
}

const jive_type_class JIVE_TYPE = {
	.parent = 0,
	.get_label = _jive_type_get_label,
	.create_input = _jive_type_create_input,
	.create_output = _jive_type_create_output,
	.create_resource = _jive_type_create_resource,
	.create_gate = _jive_type_create_gate,
	.equals = _jive_type_equals,
	.accepts = _jive_type_accepts
};

/* inputs */

const struct jive_input_class JIVE_INPUT = {
	.parent = 0,
	.fini = _jive_input_fini,
	.get_label = _jive_input_get_label,
	.get_type = _jive_input_get_type,
	.get_constraint = _jive_input_get_constraint
};

static inline void
jive_input_add_as_user(jive_input * self, jive_output * output)
{
	JIVE_LIST_PUSH_BACK(output->users, self, output_users_list);
	jive_node_add_successor(output->node);
}

static inline void
jive_input_remove_as_user(jive_input * self, jive_output * output)
{
	JIVE_LIST_REMOVE(output->users, self, output_users_list);
	jive_node_remove_successor(self->origin->node);
}

void
_jive_input_init(jive_input * self, struct jive_node * node, size_t index, jive_output * origin)
{
	DEBUG_ASSERT(jive_type_accepts(jive_input_get_type(self), jive_output_get_type(origin)));
	
	self->node = node;
	self->index = index;
	self->origin = origin;
	self->gate = 0;
	self->resource = 0;
	
	self->output_users_list.prev = self->output_users_list.next = 0;
	self->gate_inputs_list.prev = self->gate_inputs_list.next = 0;
	self->resource_input_list.prev = self->resource_input_list.next = 0;
	
	jive_input_add_as_user(self, origin);
}

void
_jive_input_fini(jive_input * self)
{
	if (self->resource) jive_resource_unassign_input(self->resource, self);
	
	if (self->gate) {
		jive_gate * gate = self->gate;
		JIVE_LIST_REMOVE(gate->inputs, self, gate_inputs_list);
		
		size_t n;
		for(n=0; n<self->node->ninputs; n++) {
			jive_input * other = self->node->inputs[n];
			if (other == self) continue;
			if (!other->gate) continue;
			size_t count = jive_gate_interference_remove(self->gate, other->gate);
			if (count == 0 && self->gate->resource && other->gate->resource)
				jive_resource_interference_remove(self->gate->resource, other->gate->resource);
		}
	}
	
	jive_input_remove_as_user(self, self->origin);
	
	size_t n;
	self->node->ninputs --;
	for(n = self->index; n<self->node->ninputs; n++) {
		self->node->inputs[n] = self->node->inputs[n+1];
		self->node->inputs[n]->index = n;
	}
	if (!self->node->ninputs)
		JIVE_LIST_PUSH_BACK(self->node->graph->top, self->node, graph_top_list);
}

char *
_jive_input_get_label(const jive_input * self)
{
	char tmp[16];
	snprintf(tmp, sizeof(tmp), "#%d", self->index);
	return strdup(tmp);
}

const jive_type *
_jive_input_get_type(const jive_input * self)
{
	return &jive_type_singleton;
}
	
jive_resource *
_jive_input_get_constraint(const jive_input * self)
{
	if (self->gate) {
		if (!self->gate->resource) {
			jive_resource * resource = jive_gate_get_constraint(self->gate);
			jive_resource_assign_gate(resource, self->gate);
		}
		return self->gate->resource;
	}
	const jive_type * type = jive_input_get_type(self);
	return jive_type_create_resource(type, self->node->graph);
}

void
jive_input_divert_origin(jive_input * self, jive_output * new_origin)
{
	DEBUG_ASSERT(jive_type_accepts(jive_input_get_type(self), jive_output_get_type(new_origin)));
	DEBUG_ASSERT(self->node->graph == new_origin->node->graph);
	
	jive_output * old_origin = self->origin;
	
	jive_input_unregister_resource_crossings(self);
	jive_input_remove_as_user(self, old_origin);
	
	self->origin = new_origin;
	jive_input_add_as_user(self, new_origin);
	
	jive_input_register_resource_crossings(self);
	jive_node_invalidate_depth_from_root(self->node);
	
	jive_graph_notify_input_change(self->node->graph, self, old_origin, new_origin);
}

void
jive_input_swap(jive_input * self, jive_input * other)
{
	DEBUG_ASSERT(jive_type_accepts(jive_input_get_type(self), jive_input_get_type(other)));
	DEBUG_ASSERT(self->node == other->node);
	
	jive_resource * r1 = self->resource;
	jive_resource * r2 = other->resource;
	
	if (r1) jive_resource_unassign_input(r1, self);
	if (r2) jive_resource_unassign_input(r2, other);
	
	jive_output * o1 = self->origin;
	jive_output * o2 = other->origin;
	
	jive_input_remove_as_user(self, o1);
	jive_input_remove_as_user(other, o2);
	
	jive_input_add_as_user(self, o2);
	jive_input_add_as_user(other, o1);
	
	self->origin = o2;
	other->origin = o1;
	
	if (r2) jive_resource_assign_input(r2, self);
	if (r1) jive_resource_assign_input(r1, other);
	
	jive_node_invalidate_depth_from_root(self->node);
	
	jive_graph_notify_input_change(self->node->graph, self, o1, o2);
	jive_graph_notify_input_change(self->node->graph, other, o2, o1);
}

static void
remove_crossed_resource_helper(void * closure, jive_node * node)
{
	jive_node_remove_crossed_resource(node, (jive_resource *) closure, 1);
}

static void
add_crossed_resource_helper(void * closure, jive_node * node)
{
	jive_node_add_crossed_resource(node, (jive_resource *) closure, 1);
}

void
jive_input_register_resource_crossings(jive_input * self)
{
	if (!self->resource) return;
	jive_input_crossing_range_apply(self, add_crossed_resource_helper, self->resource);
}

void
jive_input_unregister_resource_crossings(jive_input * self)
{
	if (!self->resource) return;
	jive_input_crossing_range_apply(self, remove_crossed_resource_helper, self->resource);
}

void
jive_input_destroy(jive_input * self)
{
	if (self->node->region) jive_graph_notify_input_destroy(self->node->graph, self);
	
	self->class_->fini(self);
	jive_context_free(self->node->graph->context, self);
}

/* outputs */

const struct jive_output_class JIVE_OUTPUT = {
	.parent = 0,
	.fini = &_jive_output_fini,
	.get_label = &_jive_output_get_label,
	.get_type = &_jive_output_get_type,
	.get_constraint = &_jive_output_get_constraint
};

void _jive_output_init(
	jive_output * self,
	struct jive_node * node,
	size_t index)
{
	self->node = node;
	self->index = index;
	self->users.first = self->users.last = 0;
	self->gate = 0;
	self->resource = 0;
	
	self->gate_outputs_list.prev = self->gate_outputs_list.next = 0;
	self->resource_output_list.prev = self->resource_output_list.next = 0;
}

void _jive_output_fini(jive_output * self)
{
	DEBUG_ASSERT(self->users.first == 0 && self->users.last == 0);
	
	if (self->resource) jive_resource_unassign_output(self->resource, self);
		
	if (self->gate) {
		jive_gate * gate = self->gate;
		JIVE_LIST_REMOVE(gate->outputs, self, gate_outputs_list);
		
		size_t n;
		for(n=0; n<self->node->noutputs; n++) {
			jive_output * other = self->node->outputs[n];
			if (other == self) continue;
			if (!other->gate) continue;
			size_t count = jive_gate_interference_remove(self->gate, other->gate);
			if (count == 0 && self->gate->resource && other->gate->resource)
				jive_resource_interference_remove(self->gate->resource, other->gate->resource);
		}
	}
	
	
	self->node->noutputs --;
	size_t n;
	for(n=self->index; n<self->node->noutputs; n++) {
		self->node->outputs[n] = self->node->outputs[n+1];
		self->node->outputs[n]->index = n;
	}
}

char *
_jive_output_get_label(const jive_output * self)
{
	char tmp[16];
	snprintf(tmp, sizeof(tmp), "#%d", self->index);
	return strdup(tmp);
}

const jive_type *
_jive_output_get_type(const jive_output * self)
{
	return &jive_type_singleton;
}
	
jive_resource *
_jive_output_get_constraint(const jive_output * self)
{
	if (self->gate) {
		if (!self->gate->resource) {
			jive_resource * resource = jive_gate_get_constraint(self->gate);
			jive_resource_assign_gate(resource, self->gate);
		}
		return self->gate->resource;
	}
	const jive_type * type = jive_output_get_type(self);
	return jive_type_create_resource(type, self->node->graph);
}

void
jive_output_replace(jive_output * self, jive_output * other)
{
	while(self->users.first) {
		jive_input * input = self->users.first;
		jive_input_divert_origin(input, other);
	}
}

void
jive_output_destroy(jive_output * self)
{
	if (self->node->region) jive_graph_notify_output_destroy(self->node->graph, self);
	
	self->class_->fini(self);
	jive_context_free(self->node->graph->context, self);
}

/* gates */

const jive_gate_class JIVE_GATE = {
	.parent = 0,
	.fini = _jive_gate_fini,
	.get_label = _jive_gate_get_label,
	.get_type = _jive_gate_get_type,
	.get_constraint = _jive_gate_get_constraint
};

void
_jive_gate_init(jive_gate * self, struct jive_graph * graph, const char name[])
{
	self->graph = graph;
	self->name = jive_context_strdup(graph->context, name);
	self->inputs.first = self->inputs.last = 0;
	self->outputs.first = self->outputs.last = 0;
	self->may_spill = true;
	self->resource = 0;
	jive_gate_interference_hash_init(&self->interference);
	self->resource_gate_list.prev = self->resource_gate_list.next = 0;
	self->graph_gate_list.prev = self->graph_gate_list.next = 0;
	
	JIVE_LIST_PUSH_BACK(graph->gates, self, graph_gate_list);
}

void
_jive_gate_fini(jive_gate * self)
{
	DEBUG_ASSERT(self->inputs.first == 0 && self->inputs.last == 0);
	DEBUG_ASSERT(self->outputs.first == 0 && self->outputs.last == 0);
	
	if (self->resource) jive_resource_unassign_gate(self->resource, self);
	
	jive_gate_interference_hash_fini(&self->interference, self->graph->context);
	jive_context_free(self->graph->context, self->name);
	
	JIVE_LIST_REMOVE(self->graph->gates, self, graph_gate_list);
}

char *
_jive_gate_get_label(const jive_gate * self)
{
	return strdup(self->name);
}

const jive_type *
_jive_gate_get_type(const jive_gate * self)
{
	return &jive_type_singleton;
}

jive_resource *
_jive_gate_get_constraint(jive_gate * self)
{
	if (self->resource) return self->resource;
	const jive_type * type = jive_gate_get_type(self);
	jive_resource * resource = jive_type_create_resource(type, self->graph);
	jive_resource_assign_gate(resource, self);
	return resource;
}

size_t
jive_gate_interferes_with(const jive_gate * self, const jive_gate * other)
{
	jive_gate_interference_part * part;
	part = jive_gate_interference_hash_lookup(&self->interference, other);
	if (part) return part->whole->count;
	else return 0;
}

void
jive_gate_destroy(jive_gate * self)
{
	self->class_->fini(self);
	jive_context_free(self->graph->context, self);
}

/* resources */

const jive_resource_class JIVE_RESOURCE = {
	.parent = 0,
	.fini = _jive_resource_fini,
	.get_label = _jive_resource_get_label,
	.get_type = _jive_resource_get_type,
	.can_merge = _jive_resource_can_merge,
	.merge = _jive_resource_merge,
	.get_cpureg = _jive_resource_get_cpureg,
	.get_regcls = _jive_resource_get_regcls,
	.get_real_regcls = _jive_resource_get_real_regcls
};

void
_jive_resource_init(jive_resource * self, jive_graph * graph)
{
	self->graph = graph;
	self->inputs.first = self->inputs.last = 0;
	self->outputs.first = self->outputs.last = 0;
	self->gates.first = self->gates.last = 0;
	
	jive_node_interaction_init(&self->node_interaction);
	jive_resource_interference_hash_init(&self->interference);
	self->hovering_region = 0;
	
	self->graph_resource_list.prev = self->graph_resource_list.next = 0;
	JIVE_LIST_PUSH_BACK(graph->unused_resources, self, graph_resource_list);
}

void
_jive_resource_fini(jive_resource * self)
{
	DEBUG_ASSERT(self->inputs.first == 0 && self->inputs.last == 0);
	DEBUG_ASSERT(self->outputs.first == 0 && self->outputs.last == 0);
	DEBUG_ASSERT(self->gates.first == 0 && self->gates.last == 0);
	jive_resource_interference_hash_fini(&self->interference, self->graph->context);
}

char *
_jive_resource_get_label(const jive_resource * self)
{
	return strdup("Resource");
}

const jive_type *
_jive_resource_get_type(const jive_resource * self)
{
	return &jive_type_singleton;
}

bool
_jive_resource_can_merge(const jive_resource * self, const jive_resource * other)
{
	if (!other) return true;
	if (self->class_ != other->class_) return false;
	if (jive_resource_interferes_with(self, other)) return false;
	return true;
}

void
_jive_resource_merge(jive_resource * self, jive_resource * other)
{
	if (!other) return;
	while(other->inputs.first) {
		jive_input * input = other->inputs.first;
		jive_resource_unassign_input(other, input);
		jive_resource_assign_input(self, input);
	}
	while(other->outputs.first) {
		jive_output * output = other->outputs.first;
		jive_resource_unassign_output(other, output);
		jive_resource_assign_output(self, output);
	}
	while(other->gates.first) {
		jive_gate * gate = other->gates.first;
		jive_resource_unassign_gate(other, gate);
		jive_resource_assign_gate(self, gate);
	}
}

const struct jive_cpureg *
_jive_resource_get_cpureg(const jive_resource * self)
{
	return 0;
}

const struct jive_regcls *
_jive_resource_get_regcls(const jive_resource * self)
{
	return 0;
}

const struct jive_regcls *
_jive_resource_get_real_regcls(const jive_resource * self)
{
	return 0;
}

bool
jive_resource_conflicts_with(const jive_resource * self, const jive_resource * other)
{
	/* TODO: must rethink python code first */
	return false;
}

bool
jive_resource_may_spill(const jive_resource * self)
{
	const jive_gate * gate = self->gates.first;
	while(gate) {
		if (!gate->may_spill) return false;
		gate = gate->resource_gate_list.next;
	}
	
	return true;
}

void
jive_resource_set_hovering_region(jive_resource * self, struct jive_region * region)
{
	jive_input * input;
	
	input = self->inputs.first;
	while(input) {
		jive_input_unregister_resource_crossings(input);
		input = input->resource_input_list.next;
	}
	
	self->hovering_region = region;
	
	input = self->inputs.first;
	while(input) {
		jive_input_unregister_resource_crossings(input);
		input = input->resource_input_list.next;
	}
}


static inline void
jive_resource_maybe_add_to_graph(jive_resource * self)
{
	if (jive_resource_used(self)) return;
	JIVE_LIST_REMOVE(self->graph->unused_resources, self, graph_resource_list);
	JIVE_LIST_PUSH_BACK(self->graph->resources, self, graph_resource_list);
}

static inline void
jive_resource_maybe_remove_from_graph(jive_resource * self)
{
	if (jive_resource_used(self)) return;
	JIVE_LIST_REMOVE(self->graph->resources, self, graph_resource_list);
	JIVE_LIST_PUSH_BACK(self->graph->unused_resources, self, graph_resource_list);
}

void
jive_resource_assign_input(jive_resource * self, jive_input * input)
{
	DEBUG_ASSERT(input->resource == 0);
	
	jive_resource_maybe_add_to_graph(self);
	
	JIVE_LIST_PUSH_BACK(self->inputs, input, resource_input_list);
	input->resource = self;
	
	jive_input_register_resource_crossings(input);
	jive_node_add_used_resource(input->node, self);
}

void
jive_resource_unassign_input(jive_resource * self, jive_input * input)
{
	DEBUG_ASSERT(input->resource == self);
	
	jive_node_remove_used_resource(input->node, self);
	jive_input_unregister_resource_crossings(input);
	
	JIVE_LIST_REMOVE(self->inputs, input, resource_input_list);
	input->resource = 0;
	
	jive_resource_maybe_remove_from_graph(self);
}

void
jive_resource_assign_output(jive_resource * self, jive_output * output)
{
	DEBUG_ASSERT(output->resource == 0);
	
	jive_resource_maybe_add_to_graph(self);
	
	JIVE_LIST_PUSH_BACK(self->outputs, output, resource_output_list);
	output->resource = self;
	
	jive_node_add_defined_resource(output->node, self);
}

void
jive_resource_unassign_output(jive_resource * self, jive_output * output)
{
	DEBUG_ASSERT(output->resource == self);
	
	jive_node_remove_defined_resource(output->node, self);
	
	JIVE_LIST_REMOVE(self->outputs, output, resource_output_list);
	output->resource = 0;
	
	jive_resource_maybe_remove_from_graph(self);
}

void
jive_resource_assign_gate(jive_resource * self, jive_gate * gate)
{
	DEBUG_ASSERT(gate->resource == 0);
	
	jive_resource_maybe_add_to_graph(self);
	
	JIVE_LIST_PUSH_BACK(self->gates, gate, resource_gate_list);
	gate->resource = self;
	
	jive_gate_interference_iterator i;
	JIVE_GATE_INTERFERENCE_ITERATE(gate->interference, i) {
		jive_gate * other = i.pos->gate;
		DEBUG_ASSERT(other != gate);
		if (other->resource)
			jive_resource_interference_add(self, other->resource);
	}
}

void
jive_resource_unassign_gate(jive_resource * self, jive_gate * gate)
{
	DEBUG_ASSERT(gate->resource == self);
	
	JIVE_LIST_REMOVE(self->gates, gate, resource_gate_list);
	gate->resource = 0;
	
	jive_resource_maybe_remove_from_graph(self);
	
	jive_gate_interference_iterator i;
	JIVE_GATE_INTERFERENCE_ITERATE(gate->interference, i) {
		jive_gate * other = i.pos->gate;
		DEBUG_ASSERT(other != gate);
		if (other->resource)
			jive_resource_interference_remove(self, other->resource);
	}
}

void
jive_resource_destroy(jive_resource * self)
{
	self->class_->fini(self);
	while(self->inputs.first) jive_resource_unassign_input(self, self->inputs.first);
	while(self->outputs.first) jive_resource_unassign_output(self, self->outputs.first);
	while(self->gates.first) jive_resource_unassign_gate(self, self->gates.first);
	JIVE_LIST_REMOVE(self->graph->unused_resources, self, graph_resource_list);
	jive_context_free(self->graph->context, self);
}

size_t
jive_resource_is_active_before(const jive_resource * self, const struct jive_node * node)
{
	jive_node_resource_interaction * xpoint;
	xpoint = jive_node_resource_interaction_lookup(node, self);
	if (xpoint) return xpoint->before_count;
	else return 0;
}

size_t
jive_resource_crosses(const jive_resource * self, const struct jive_node * node)
{
	jive_node_resource_interaction * xpoint;
	xpoint = jive_node_resource_interaction_lookup(node, self);
	if (xpoint) return xpoint->crossed_count;
	else return 0;
}

size_t
jive_resource_is_active_after(const jive_resource * self, const struct jive_node * node)
{
	jive_node_resource_interaction * xpoint;
	xpoint = jive_node_resource_interaction_lookup(node, self);
	if (xpoint) return xpoint->after_count;
	else return 0;
}

size_t
jive_resource_originates_in(const jive_resource * self, const struct jive_node * node)
{
	jive_node_resource_interaction * xpoint;
	xpoint = jive_node_resource_interaction_lookup(node, self);
	if (xpoint) return xpoint->after_count - xpoint->crossed_count;
	else return 0;
}

size_t
jive_resource_is_used_by(const jive_resource * self, const struct jive_node * node)
{
	jive_node_resource_interaction * xpoint;
	xpoint = jive_node_resource_interaction_lookup(node, self);
	if (xpoint) return xpoint->before_count - xpoint->crossed_count;
	else return 0;
}

size_t
jive_resource_interferes_with(const jive_resource * self, const jive_resource * other)
{
	jive_resource_interference_part * part;
	part = jive_resource_interference_hash_lookup(&self->interference, other);
	if (part) return part->whole->count;
	else return 0;
}
