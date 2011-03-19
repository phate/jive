#include <jive/vsdg/region.h>

#include <jive/arch/stackframe.h>
#include <jive/debug-private.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/region-ssavar-use-private.h>
#include <jive/vsdg/variable.h>
#include <jive/util/list.h>

void
_jive_region_init(jive_region * self, jive_graph * graph, jive_region * parent)
{
	self->graph = graph;
	self->parent = parent;
	self->stackframe = 0;
	
	self->nodes.first = self->nodes.last = 0;
	self->subregions.first = self->subregions.last = 0;
	self->region_subregions_list.prev = self->region_subregions_list.next = 0;
	self->is_looped = true;
	jive_region_ssavar_hash_init(&self->used_ssavars, graph->context);
	
	if (parent) {
		JIVE_LIST_PUSH_BACK(parent->subregions, self, region_subregions_list);
		self->depth = parent->depth + 1;
	} else self->depth = 0;
	
	self->anchor_node = 0;
	
	jive_graph_notify_region_create(graph, self);
}

void
_jive_region_fini(jive_region * self)
{
	DEBUG_ASSERT(jive_region_empty(self));
	DEBUG_ASSERT(self->nodes.first == 0 && self->nodes.last == 0);
	DEBUG_ASSERT(self->subregions.first == 0 && self->subregions.last == 0);
	
	jive_graph_notify_region_destroy(self->graph, self);
	
	jive_region_ssavar_hash_fini(&self->used_ssavars);
	if (self->parent)
		JIVE_LIST_REMOVE(self->parent->subregions, self, region_subregions_list);
	/* FIXME: destroy stackframe! */
	/* if (self->stackframe)
		jive_stackframe_destroy(self->stackframe); */
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

jive_region *
jive_region_create_subregion(jive_region * self)
{
	jive_region * subregion = jive_context_malloc(self->graph->context, sizeof(*subregion));
	_jive_region_init(subregion, self->graph, self);
	return subregion;
}

void
jive_region_add_used_ssavar(jive_region * self, jive_ssavar * ssavar)
{
	if (ssavar->origin->node->region->depth >= self->depth) return;
	if (self->is_looped) {
		jive_region_ssavar_use * use;
		use = jive_region_ssavar_hash_lookup(&self->used_ssavars, ssavar);
		if (use)
			use->count ++;
		else {
			use = jive_context_malloc(self->graph->context, sizeof(*use));
			use->region = self;
			use->ssavar = ssavar;
			use->count = 1;
			jive_region_ssavar_hash_insert(&self->used_ssavars, use);
			jive_ssavar_region_hash_insert(&ssavar->assigned_regions, use);
			
			jive_graph_notify_region_add_used_ssavar(self->graph, self, ssavar);
		}
	}
	
	jive_region_add_used_ssavar(self->parent, ssavar);
}

void
jive_region_remove_used_ssavar(jive_region * self, jive_ssavar * ssavar)
{
	if (ssavar->origin->node->region->depth >= self->depth) return;
	if (self->is_looped) {
		jive_region_ssavar_use * use;
		use = jive_region_ssavar_hash_lookup(&self->used_ssavars, ssavar);
		use->count --;
		if (use->count == 0) {
			jive_region_ssavar_hash_remove(&self->used_ssavars, use);
			jive_ssavar_region_hash_remove(&ssavar->assigned_regions, use);
			
			jive_graph_notify_region_remove_used_ssavar(self->graph, self, ssavar);
		}
	}
	
	jive_region_add_used_ssavar(self->parent, ssavar);
}

#if 0
	def add_used_ssavar(self, ssavar):
		if ssavar.origin.node.region.depth >= self.depth: return
		if self.is_loop_region:
			count = self.used_ssavars.add(ssavar, 1)
			if count == 0:
				ssavar.assigned_regions.add(self)
				self.graph.on_region_add_used_ssavar(self, ssavar)
			
		self.parent.add_used_ssavar(ssavar)
	
	def remove_used_ssavar(self, ssavar):
		if ssavar.origin.node.region.depth >= self.depth: return
		if self.is_loop_region:
			count = self.used_ssavars.remove(ssavar, 1)
			if count == 0:
				ssavar.assigned_regions.remove(self)
				self.graph.on_region_remove_used_ssavar(self, ssavar)
		self.parent.add_used_ssavar(ssavar)
#endif
