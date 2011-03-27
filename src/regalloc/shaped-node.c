#include <jive/regalloc/shaped-node.h>
#include <jive/regalloc/shaped-node-private.h>

#include <jive/context.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/xpoint-private.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>

JIVE_DEFINE_HASH_TYPE(jive_shaped_node_hash, jive_shaped_node, struct jive_node *, node, hash_chain);

jive_shaped_node *
jive_shaped_node_create(jive_cut * cut, struct jive_node * node)
{
	jive_shaped_graph * shaped_graph = cut->shaped_region->shaped_graph;
	JIVE_DEBUG_ASSERT(!node->region->anchor || jive_shaped_graph_map_node(shaped_graph, node->region->anchor->node));
	jive_context * context = shaped_graph->context;
	jive_shaped_node * self = jive_context_malloc(context, sizeof(*self));
	
	self->shaped_graph = shaped_graph;
	self->node = node;
	
	jive_shaped_node_hash_insert(&shaped_graph->node_map, self);
	
	self->cut = cut;
	
	jive_ssavar_xpoint_hash_init(&self->ssavar_xpoints, context);
	
	return self;
}

jive_shaped_node *
jive_shaped_node_prev_in_region(const jive_shaped_node * self)
{
	jive_shaped_node * tmp = self->cut_location_list.prev;
	jive_cut * cut = self->cut->region_cut_list.prev;
	
	while(cut && !tmp) {
		tmp = cut->locations.last;
		cut = cut->region_cut_list.prev;
	}
	
	return tmp;
}

jive_shaped_node *
jive_shaped_node_next_in_region(const jive_shaped_node * self)
{
	jive_shaped_node * tmp = self->cut_location_list.next;
	jive_cut * cut = self->cut->region_cut_list.next;
	
	while(cut && !tmp) {
		tmp = cut->locations.first;
		cut = cut->region_cut_list.next;
	}
	
	return tmp;
}

static void
jive_shaped_node_remove_all_crossed(jive_shaped_node * self)
{
	struct jive_ssavar_xpoint_hash_iterator i;
	i = jive_ssavar_xpoint_hash_begin(&self->ssavar_xpoints);
	while(i.entry) {
		jive_xpoint * xpoint = i.entry;
		jive_ssavar_xpoint_hash_iterator_next(&i);
		
		jive_shaped_node_remove_ssavar_crossed(self, xpoint->shaped_ssavar, xpoint->shaped_ssavar->ssavar->variable, xpoint->cross_count);
	}
}

void
jive_shaped_node_destroy(jive_shaped_node * self)
{
	/* destroy node shapes of all anchored regions */
	size_t n;
	for(n = 0; n < self->node->ninputs; n++)
	{
		jive_input * input = self->node->inputs[n];
		if (jive_input_isinstance(input, &JIVE_CONTROL_INPUT)) {
			jive_region * region = input->origin->node->region;
			jive_shaped_region * shaped_region = jive_shaped_graph_map_region(self->shaped_graph, region);
			jive_shaped_region_destroy_cuts(shaped_region);
		}
	}
	
	/* remove things that cross this node */
	jive_shaped_node_remove_all_crossed(self);
	
	/* set aside crossings of vars beginning or ending here */
	for(n = 0; n < self->node->ninputs; n++) {
		jive_input * input = self->node->inputs[n];
		jive_ssavar * ssavar = input->ssavar;
		if (!ssavar) continue;
		jive_shaped_ssavar_xpoints_unregister_arc(jive_shaped_graph_map_ssavar(self->shaped_graph, ssavar), input, input->origin);
	}
	for(n = 0; n < self->node->noutputs; n++) {
		jive_output * output = self->node->outputs[n];
		jive_input * user;
		JIVE_LIST_ITERATE(output->users, user, output_users_list) {
			jive_ssavar * ssavar = user->ssavar;
			if (!ssavar) continue;
			jive_shaped_ssavar_xpoints_unregister_arc(jive_shaped_graph_map_ssavar(self->shaped_graph, ssavar), user, output);
		}
		jive_ssavar * ssavar = output->ssavar;
		if (!ssavar) continue;
		jive_shaped_node_remove_ssavar_after(self, jive_shaped_graph_map_ssavar(self->shaped_graph, ssavar), ssavar->variable, 1);
	}
	
	/* remove from graph */
	jive_shaped_node_hash_remove(&self->shaped_graph->node_map, self);
	JIVE_LIST_REMOVE(self->cut->locations, self, cut_location_list);
	
	/* reinstate crossings for those arcs that have this node as origin */
	for(n = 0; n < self->node->noutputs; n++) {
		jive_output * output = self->node->outputs[n];
		jive_input * user;
		JIVE_LIST_ITERATE(output->users, user, output_users_list) {
			jive_ssavar * ssavar = user->ssavar;
			if (!ssavar) continue;
			jive_shaped_ssavar_xpoints_register_arc(jive_shaped_graph_map_ssavar(self->shaped_graph, ssavar), user, output);
		}
	}
	
	JIVE_DEBUG_ASSERT(self->ssavar_xpoints.nitems == 0);
	jive_ssavar_xpoint_hash_fini(&self->ssavar_xpoints);
	jive_context_free(self->shaped_graph->context, self);
}
