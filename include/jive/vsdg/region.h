/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_REGION_H
#define JIVE_VSDG_REGION_H

#include <stdbool.h>
#include <stddef.h>

#include <jive/common.h>
#include <jive/util/list.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/region-ssavar-use.h>
#include <jive/vsdg/section.h>

namespace jive {
	class input;
	class substitution_map;
}

typedef struct jive_region jive_region;

struct jive_cut;
struct jive_graph;
struct jive_node;
struct jive_stackframe;

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
};

typedef struct jive_region_hull_entry jive_region_hull_entry;

struct jive_region_hull_entry {
	struct {
		struct jive_region_hull_entry * prev;
		struct jive_region_hull_entry * next;
	} region_hull_list;

	struct {
		struct jive_region_hull_entry * prev;
		struct jive_region_hull_entry * next;
	} input_hull_list;

	/* the input that the entry represents */
	jive::input * input;

	/* the region where entry lives */
	struct jive_region * region;
};

class jive_region {
public:
	~jive_region();

	jive_region(jive_region * parent, jive_graph * graph);

	void reparent(jive_region * new_parent) noexcept;

	inline size_t depth() const noexcept
	{
		return depth_;
	}

	inline bool
	contains(const jive_region * other) const noexcept
	{
		while (other->depth() > depth()) {
			if (other->parent == this)
				return true;
			other = other->parent;
		}
		return false;
	}

	inline bool
	is_empty() const noexcept
	{
		return nodes.first == nullptr && subregions.first == nullptr;
	}

	struct jive_graph * graph;
	jive_region * parent;
	struct jive_stackframe * stackframe;

	struct {
		struct jive_node * first;
		struct jive_node * last;
	} nodes;

	struct {
		struct jive_node * first;
		struct jive_node * last;
	} top_nodes;

	struct {
		jive_region * first;
		jive_region * last;
	} subregions;
	struct {
		jive_region * prev;
		jive_region * next;
	} region_subregions_list;
	struct {
		struct jive_region_hull_entry * first;
		struct jive_region_hull_entry * last;
	} hull;
	
	jive_region_attrs attrs;
	
	struct jive_node * top;
	struct jive_node * bottom;
	
	jive_region_ssavar_hash used_ssavars;
	
	struct jive::input * anchor;

private:
	size_t depth_;
};

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
jive_region_copy_substitute(
	const jive_region * self,
	jive_region * target,
	jive::substitution_map & substitution,
	bool copy_top, bool copy_bottom);

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

struct jive_node *
jive_region_get_anchor(struct jive_region * self);

bool
jive_region_depends_on_region(const jive_region * self, const jive_region * region);

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

void
jive_region_hull_add_input(struct jive_region * region, jive::input * input);

JIVE_EXPORTED_INLINE void
jive_region_hull_add_hull_to_parents(struct jive_region * region)
{
	jive_region_hull_entry * entry, * next;
	JIVE_LIST_ITERATE_SAFE(region->hull, entry, next, region_hull_list)
		jive_region_hull_add_input(region->parent, entry->input);
}

void
jive_region_hull_remove_input(struct jive_region * region, jive::input * input);

JIVE_EXPORTED_INLINE void
jive_region_hull_remove_hull_from_parents(struct jive_region * region)
{
	jive_region_hull_entry * entry, * next;
	JIVE_LIST_ITERATE_SAFE(region->hull, entry, next, region_hull_list)
		jive_region_hull_remove_input(region->parent, entry->input);
}

#ifdef JIVE_DEBUG
void
jive_region_verify_hull(struct jive_region * region);

void
jive_region_verify_top_node_list(struct jive_region * region);
#endif

#endif
