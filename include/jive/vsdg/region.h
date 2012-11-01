/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_REGION_H
#define JIVE_VSDG_REGION_H

#include <stdbool.h>
#include <stddef.h>

#include <jive/vsdg/section.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/region-ssavar-use.h>

typedef struct jive_region jive_region;

struct jive_graph;
struct jive_input;
struct jive_node;
struct jive_cut;
struct jive_stackframe;
struct jive_substitution_map;

typedef struct jive_region_attrs jive_region_attrs;

typedef enum jive_region_section_flags {
	jive_region_section_inherit = 0,
	jive_region_section_code = 1,
	jive_region_section_data = 2,
	jive_region_section_rodata = 3,
	jive_region_section_bss = 4
} jive_region_section_flags;

struct jive_region_attrs {
	size_t align;
	jive_region_section_flags section;
	bool is_looped;
	bool is_floating;
};

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
	
	jive_region_attrs attrs;
	
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

JIVE_EXPORTED_INLINE struct jive_node *
jive_region_get_top_node(jive_region * self)
{
	return self->top;
}

JIVE_EXPORTED_INLINE struct jive_node *
jive_region_get_bottom_node(jive_region * self)
{
	return self->bottom;
}

jive_region *
jive_region_create_subregion(jive_region * self);

struct jive_region *
jive_floating_region_create(struct jive_graph * graph);

bool
jive_region_depends_on_region(const jive_region * self, const jive_region * region);

void
jive_region_reparent(jive_region * self, jive_region * new_parent);

jive_region *
jive_region_compute_outermost_parent(const jive_region * self);

JIVE_EXPORTED_INLINE struct jive_stackframe *
jive_region_get_stackframe(const jive_region * region)
{
	while(region && !region->stackframe) region = region->parent;
	if (region) return region->stackframe;
	else return 0;
}

JIVE_EXPORTED_INLINE jive_stdsectionid
jive_region_map_to_section(const struct jive_region * region)
{
	while (region && (region->attrs.section == jive_region_section_inherit))
		region = region->parent;

	jive_stdsectionid section = jive_stdsectionid_invalid;
	if (region) {
		switch(region->attrs.section) {
			case jive_region_section_code:
				section = jive_stdsectionid_code;
				break;
			case jive_region_section_data:
				section = jive_stdsectionid_data;
				break;
			case jive_region_section_rodata:
				section = jive_stdsectionid_rodata;
				break;
			case jive_region_section_bss:
				section = jive_stdsectionid_bss;
				break;
			default:
				JIVE_DEBUG_ASSERT(0);
		}
	}
	
	return section;
}

#endif
