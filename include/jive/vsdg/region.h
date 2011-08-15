#ifndef JIVE_VSDG_REGION_H
#define JIVE_VSDG_REGION_H

#include <stdbool.h>
#include <stddef.h>

#include <jive/vsdg/basetype.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region-ssavar-use.h>

typedef struct jive_region jive_region;

struct jive_graph;
struct jive_input;
struct jive_node;
struct jive_cut;
struct jive_stackframe;
struct jive_substitution_map;

struct jive_region {
	struct jive_graph * graph;
	jive_region * parent;
	size_t depth;
	struct jive_stackframe * stackframe;
	struct {
		struct jive_node * first;
		struct jive_node * last;
	} nodes;
	struct {
		jive_region * first;
		jive_region * last;
	} subregions;
	struct {
		jive_region * prev;
		jive_region * next;
	} region_subregions_list;
	
	bool is_looped;
	
	struct jive_node * top;
	struct jive_node * bottom;
	
	jive_region_ssavar_hash used_ssavars;
	
	struct jive_input * anchor;
};

void
jive_region_destroy(jive_region * self);


/**
	\brief Copy a region with substitutions
	\param self Region to be copied
	\param target Target region to create nodes in
	\param substitution Operand and gate substitutions
	\param copy_top Copy top node of region
	\param copy_bottom Copy bottom node of region
	
	Copies all nodes of the specified region and its
	subregions into the target region. Substitutions
	will be performed as specified, and the substitution
	map will be updated as nodes are copied.
	
	@c self must be a region with uniquely determined top
	and bottom nodes. Optionally, these nodes are copied
	as well.
*/
void
jive_region_copy_substitute(const jive_region * self, jive_region * target,
	struct jive_substitution_map * substitution,
	bool copy_top, bool copy_bottom);

JIVE_EXPORTED_INLINE bool
jive_region_empty(const jive_region * self)
{
	return self->nodes.first == 0 && self->subregions.first == 0;
}

JIVE_EXPORTED_INLINE bool
jive_region_is_contained_by(const jive_region * self, const jive_region * other)
{
	while(self->depth > other->depth) {
		if (self->parent == other) return true;
		self = self->parent;
	}
	return false;
}

JIVE_EXPORTED_INLINE bool
jive_region_contains_node(const jive_region * self, const jive_node * node)
{
	const jive_region * tmp = node->region;
	while(tmp->depth >= self->depth) {
		if (tmp == self) return true;
		tmp = tmp->parent;
		if (!tmp) break;
	}
	return false;
}

JIVE_EXPORTED_INLINE jive_node *
jive_region_get_top_node(jive_region * self)
{
	return self->top;
}

JIVE_EXPORTED_INLINE jive_node *
jive_region_get_bottom_node(jive_region * self)
{
	return self->bottom;
}

jive_region *
jive_region_create_subregion(jive_region * self);

bool
jive_region_depends_on_region(const jive_region * self, const jive_region * region);

void
jive_region_reparent(jive_region * self, jive_region * new_parent);

JIVE_EXPORTED_INLINE struct jive_stackframe *
jive_region_get_stackframe(const jive_region * region)
{
	while(region && !region->stackframe) region = region->parent;
	if (region) return region->stackframe;
	else return 0;
}

/** \brief Determine innermost of multiple (possibly) nested regions from operand list */
JIVE_EXPORTED_INLINE jive_region *
jive_region_innermost(size_t noperands, jive_output * const operands[])
{
	jive_region * region = operands[0]->node->region;
	size_t n;
	for(n = 1; n < noperands; n++) {
		if (operands[n]->node->region->depth > region->depth)
			region = operands[n]->node->region;
	}
	
	return region;
}

#endif
