#include <jive/vsdg/cut-private.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/crossings-private.h>

#include <jive/context.h>
#include <jive/util/list.h>
#include <jive/debug-private.h>

void
jive_cut_destroy(jive_cut * self)
{
	while(self->nodes.first) jive_node_location_destroy(self->nodes.first);
	jive_context_free(self->region->graph->context, self);
}

void
jive_node_location_destroy(jive_node_location * self)
{
	jive_node * node = self->node;
	jive_node_unregister_resource_crossings(node);
	JIVE_LIST_REMOVE(self->cut->nodes, self, cut_nodes_list);
	self->node->shape_location = 0;
	jive_context_free(node->graph->context, self);
	jive_node_register_resource_crossings(node);
}

static inline void
jive_node_location_init(jive_node_location * self, jive_cut * cut, jive_node * node)
{
	self->node = node;
	self->cut = cut;
	self->cut_nodes_list.prev = self->cut_nodes_list.next = 0;
}

jive_node_location *
jive_cut_append(jive_cut * self, jive_node * node)
{
	return jive_cut_insert(self, 0, node);
}

jive_node_location *
jive_cut_insert(jive_cut * self, jive_node_location * at, struct jive_node * node)
{
	DEBUG_ASSERT(node->shape_location == 0);
	
	/* temporarily set aside input/output resource crossings */
	jive_node_unregister_resource_crossings(node);
	
	/* create and setup location, link into cut */
	jive_node_location * loc = jive_context_malloc(node->graph->context, sizeof(*loc));
	jive_node_location_init(loc, self, node);
	JIVE_LIST_INSERT(self->nodes, at, loc, cut_nodes_list);
	node->shape_location = loc;
	
	/* determine crossed resources */
	jive_node_location * next = jive_node_location_next_in_region(loc);
	if (next) {
		jive_node * next_node = next->node;
		
		/* resources that cross the next node must also cross this node
		(we have temporarily set the resources originating in our node
		aside) */
		jive_resource_interaction_iterator i;
		JIVE_RESOURCE_INTERACTION_ITERATE(next_node->resource_interaction, i) {
			jive_node_resource_interaction * xpoint = i.pos;
			if (xpoint->crossed_count == 0) continue;
			jive_node_add_crossed_resource(node, xpoint->resource, xpoint->crossed_count);
		}
		
		/* resources that are inputs to he next node must also cross this
		node, unless they originate here (or are exempt from crossing for
		other reasions) */
		size_t n;
		for(n=0; n<next_node->ninputs; n++) {
			jive_input * input = next_node->inputs[n];
			/* TODO: treat ControlType inputs */
			
			if (input->origin->node == node) continue;
			if (!input->resource) continue;
			if (input->resource->hovering_region || input->origin->node->shape_location) {
				jive_node_add_crossed_resource(node, input->resource, 1);
			}
		}
	} else if (node->region->anchor_node) {
		/* TODO */
		abort();
	}
	
	/* finally, reinstate input/output resource crossings */
	jive_node_register_resource_crossings(node);
	return loc;
}

jive_node_location *
jive_node_location_next_in_region_slow(const jive_node_location * self)
{
	jive_cut * cut = self->cut->region_cuts_list.next;
	/* skip empty cuts; they are only "slightly" legal,
	but have to cope with them nevertheless */
	while(cut && jive_cut_empty(cut)) cut = cut ->region_cuts_list.next;
	if (cut) return cut->nodes.first;
	return 0;
}

