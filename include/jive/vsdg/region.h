/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_REGION_H
#define JIVE_VSDG_REGION_H

#include <stdbool.h>
#include <stddef.h>

#include <jive/common.h>
#include <jive/util/list.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/section.h>

namespace jive {
	class input;
	class substitution_map;
}

typedef struct jive_region jive_region;

struct jive_cut;
struct jive_graph;
struct jive_node;

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
			if (other->parent() == this)
				return true;
			other = other->parent();
		}
		return false;
	}

	bool
	contains(const jive_node * node) const noexcept;

	inline jive_region *
	parent() const noexcept
	{
		return parent_;
	}

	inline jive_graph *
	graph() const noexcept
	{
		return graph_;
	}

	inline jive_node *
	top() const noexcept
	{
		return top_;
	}

	/*
		FIXME: this is going to be removed again
	*/
	inline void
	set_top(jive_node * top) noexcept
	{
		top_ = top;
	}

	inline jive_node *
	bottom() const noexcept
	{
		return bottom_;
	}

	/*
		FIXME: this is going to be removed again
	*/
	inline void
	set_bottom(jive_node * bottom) noexcept
	{
		bottom_ = bottom;
	}

	inline jive::input *
	anchor() const noexcept
	{
		return anchor_;
	}

	/*
		FIXME: this is going to be removed again
	*/
	inline void
	set_anchor(jive::input * anchor) noexcept
	{
		anchor_ = anchor;
	}

	typedef jive::detail::intrusive_list<
		jive_node,
		jive_node::region_node_list_accessor
	> region_nodes_list;

	region_nodes_list nodes;

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

	jive_region_attrs attrs;


private:
	size_t depth_;
	jive_node * top_;
	jive_node * bottom_;
	jive_graph * graph_;
	jive_region * parent_;
	jive::input * anchor_;
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

JIVE_EXPORTED_INLINE jive_stdsectionid
jive_region_map_to_section(const struct jive_region * region)
{
	while (region && (region->attrs.section == jive_region_section_inherit))
		region = region->parent();

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

#ifdef JIVE_DEBUG
void
jive_region_verify_top_node_list(struct jive_region * region);
#endif

#endif
