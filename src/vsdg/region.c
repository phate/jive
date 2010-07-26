#include <jive/vsdg/region.h>
#include <jive/vsdg/graph.h>
#include <jive/util/list.h>
#include <jive/debug-private.h>

void
_jive_region_init(jive_region * self, jive_graph * graph, jive_region * parent)
{
	self->graph = graph;
	self->parent = parent;
	
	self->nodes.first = self->nodes.last = 0;
	self->subregions.first = self->subregions.last = 0;
	self->region_subregions_list.prev = self->region_subregions_list.next = 0;
	
	if (parent)
		JIVE_LIST_PUSHBACK(parent->subregions, self, region_subregions_list);
	
	self->top_cut = self->bottom_cut = 0;
	self->anchor_node = 0;
}

void
_jive_region_fini(jive_region * self)
{
	DEBUG_ASSERT(self->nodes.first == 0 && self->nodes.last == 0);
	if (self->parent)
		JIVE_LIST_REMOVE(self->parent->subregions, self, region_subregions_list);
}
