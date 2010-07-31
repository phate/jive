#include <jive/vsdg/cut-private.h>
#include <jive/vsdg/node-private.h>

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
	jive_node_unregister_resource_crossings(self->node);
	JIVE_LIST_REMOVE(self->cut->nodes, self, cut_nodes_list);
	self->node->shape_location = 0;
	jive_context_free(self->node->graph->context, self);
	jive_node_register_resource_crossings(self->node);
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
	jive_node_unregister_resource_crossings(node);
	DEBUG_ASSERT(node->shape_location == 0);
	jive_node_location * loc = jive_context_malloc(node->graph->context, sizeof(*loc));
	jive_node_location_init(loc, self, node);
	JIVE_LIST_INSERT(self->nodes, at, loc, cut_nodes_list);
	node->shape_location = loc;
	jive_node_register_resource_crossings(node);
	return loc;
}
