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

struct jive_cut;
struct jive_graph;
struct jive_node;

namespace jive {

class region {
public:
	~region();

	region(jive::region * parent, jive_graph * graph);

	void reparent(jive::region * new_parent) noexcept;

	inline size_t depth() const noexcept
	{
		return depth_;
	}

	inline bool
	contains(const jive::region * other) const noexcept
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

	inline jive::region *
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
		jive::region * first;
		jive::region * last;
	} subregions;
	struct {
		jive::region * prev;
		jive::region * next;
	} region_subregions_list;

private:
	size_t depth_;
	jive_node * top_;
	jive_node * bottom_;
	jive_graph * graph_;
	jive::region * parent_;
	jive::input * anchor_;
};

} //namespace

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
	const jive::region * self,
	jive::region * target,
	jive::substitution_map & substitution,
	bool copy_top, bool copy_bottom);

#ifdef JIVE_DEBUG
void
jive_region_verify_top_node_list(struct jive::region * region);
#endif

#endif
