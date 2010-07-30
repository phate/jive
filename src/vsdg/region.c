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
		JIVE_LIST_PUSH_BACK(parent->subregions, self, region_subregions_list);
	
	self->top_cut = self->bottom_cut = 0;
	self->anchor_node = 0;
}

void
_jive_region_fini(jive_region * self)
{
	DEBUG_ASSERT(jive_region_empty(self));
	DEBUG_ASSERT(self->nodes.first == 0 && self->nodes.last == 0);
	DEBUG_ASSERT(self->subregions.first == 0 && self->subregions.last == 0);
	if (self->parent)
		JIVE_LIST_REMOVE(self->parent->subregions, self, region_subregions_list);
}

void
jive_region_destroy(jive_region * self)
{
	_jive_region_fini(self);
	jive_context_free(self->graph->context, self);
}

void
_jive_region_prune_subregions(jive_region * self)
{
	jive_region * subregion;
	subregion = self->subregions.first;
	while(subregion) {
		jive_region * next = subregion->region_subregions_list.next;
		if (jive_region_empty(subregion)) {
			JIVE_LIST_REMOVE(self->subregions, subregion, region_subregions_list);
			jive_region_destroy(subregion);
		}
		subregion = next;
	}
}
