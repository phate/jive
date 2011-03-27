#include <jive/regalloc/shaped-variable.h>
#include <jive/regalloc/shaped-variable-private.h>

#include <jive/common.h>
#include <jive/context.h>
#include <jive/regalloc/crossing-arc.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/xpoint-private.h>
#include <jive/regalloc/shaped-node-private.h>
#include <jive/regalloc/shaped-region-private.h>

JIVE_DEFINE_HASH_TYPE(jive_shaped_variable_hash, jive_shaped_variable, struct jive_variable *, variable, hash_chain);

jive_shaped_variable *
jive_shaped_variable_create(struct jive_shaped_graph * shaped_graph, struct jive_variable * variable)
{
	jive_context * context = shaped_graph->context;
	jive_shaped_variable * self = jive_context_malloc(context, sizeof(*self));
	
	self->shaped_graph = shaped_graph;
	self->variable = variable;
	
	jive_shaped_variable_hash_insert(&shaped_graph->variable_map, self);
	jive_variable_interference_hash_init(&self->interference, context);
	
	return self;
}

void
jive_shaped_variable_destroy(jive_shaped_variable * self)
{
	JIVE_DEBUG_ASSERT(self->interference.nitems == 0);
	jive_variable_interference_hash_fini(&self->interference);
	jive_shaped_variable_hash_remove(&self->shaped_graph->variable_map, self);
	jive_context_free(self->shaped_graph->context, self);
}

jive_variable_interference *
jive_variable_interference_create(jive_shaped_variable * first, jive_shaped_variable * second)
{
	jive_variable_interference * i = jive_context_malloc(first->shaped_graph->context, sizeof(*i));
	i->first.shaped_variable = first;
	i->first.whole = i;
	i->second.shaped_variable = second;
	i->second.whole = i;
	i->count = 0;
	
	jive_variable_interference_hash_insert(&first->interference, &i->second);
	jive_variable_interference_hash_insert(&second->interference, &i->first);
	
	return i;
}

void
jive_variable_interference_destroy(jive_variable_interference * self)
{
	jive_variable_interference_hash_remove(&self->first.shaped_variable->interference, &self->second);
	jive_variable_interference_hash_remove(&self->second.shaped_variable->interference, &self->first);
	jive_context_free(self->first.shaped_variable->shaped_graph->context, self);
}


JIVE_DEFINE_HASH_TYPE(jive_shaped_ssavar_hash, jive_shaped_ssavar, struct jive_ssavar *, ssavar, hash_chain);

jive_shaped_ssavar *
jive_shaped_ssavar_create(struct jive_shaped_graph * shaped_graph, struct jive_ssavar * ssavar)
{
	jive_context * context = shaped_graph->context;
	jive_shaped_ssavar * self = jive_context_malloc(context, sizeof(*self));
	
	self->shaped_graph = shaped_graph;
	self->ssavar = ssavar;
	self->hovering = false;
	
	jive_shaped_ssavar_hash_insert(&shaped_graph->ssavar_map, self);
	
	jive_node_xpoint_hash_init(&self->node_xpoints, context);
	
	return self;
}

size_t
jive_shaped_variable_interferes_with(const jive_shaped_variable * self, const jive_shaped_variable * other)
{
	jive_variable_interference_part * part = jive_variable_interference_hash_lookup(&self->interference, other);
	if (part)
		return part->whole->count;
	else
		return 0;
}

void
jive_shaped_ssavar_destroy(jive_shaped_ssavar * self)
{
	JIVE_DEBUG_ASSERT(self->node_xpoints.nitems == 0);
	jive_node_xpoint_hash_fini(&self->node_xpoints);
	jive_shaped_ssavar_hash_remove(&self->shaped_graph->ssavar_map, self);
	jive_context_free(self->shaped_graph->context, self);
}

void
jive_shaped_ssavar_xpoints_register_arc(jive_shaped_ssavar * self, jive_input * input, jive_output * output)
{
	jive_variable * variable = self->ssavar->variable;
	
	jive_shaped_node * origin_shaped_node = jive_shaped_graph_map_node(self->shaped_graph, output->node);
	jive_shaped_node * input_shaped_node = jive_shaped_graph_map_node(self->shaped_graph, input->node);
	
	jive_crossing_arc_iterator i;
	jive_crossing_arc_iterator_init(&i, origin_shaped_node, input_shaped_node, self);
	
	while(i.region) {
		if (i.node)
			jive_shaped_node_add_ssavar_crossed(i.node, self, variable, 1);
		else
			jive_shaped_region_add_active_top(i.region, self);
		jive_crossing_arc_iterator_next(&i);
	}
	
	if (input_shaped_node && (origin_shaped_node || self->hovering))
		jive_shaped_node_add_ssavar_before(input_shaped_node, self, variable, 1);
	
	if (origin_shaped_node && input_shaped_node)
		jive_shaped_node_add_ssavar_after(origin_shaped_node, self, variable, 1);
}

void
jive_shaped_ssavar_xpoints_unregister_arc(jive_shaped_ssavar * self, jive_input * input, jive_output * output)
{
	jive_variable * variable = self->ssavar->variable;
	
	jive_shaped_node * origin_shaped_node = jive_shaped_graph_map_node(self->shaped_graph, output->node);
	jive_shaped_node * input_shaped_node = jive_shaped_graph_map_node(self->shaped_graph, input->node);
	
	jive_crossing_arc_iterator i;
	jive_crossing_arc_iterator_init(&i, origin_shaped_node, input_shaped_node, self);
	
	while(i.region) {
		if (i.node)
			jive_shaped_node_remove_ssavar_crossed(i.node, self, variable, 1);
		else
			jive_shaped_region_remove_active_top(i.region, self);
		jive_crossing_arc_iterator_next(&i);
	}
	
	if (input_shaped_node && (origin_shaped_node || self->hovering))
		jive_shaped_node_remove_ssavar_before(input_shaped_node, self, variable, 1);
	
	if (origin_shaped_node && input_shaped_node)
		jive_shaped_node_remove_ssavar_after(origin_shaped_node, self, variable, 1);
}
