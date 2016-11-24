/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/region.h>

#include <jive/common.h>

#include <jive/util/list.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/substitution.h>
#include <jive/vsdg/traverser.h>

namespace jive {

region::~region()
{
	JIVE_DEBUG_ASSERT(nodes.empty());
	JIVE_DEBUG_ASSERT(subregions.first == nullptr && subregions.last == nullptr);

	graph_->on_region_destroy(this);

	if (parent_)
		JIVE_LIST_REMOVE(parent_->subregions, this, region_subregions_list);
}

region::region(jive::region * parent, jive_graph * graph)
	: depth_(0)
	, top_(nullptr)
	, bottom_(nullptr)
	, graph_(graph)
	, parent_(parent)
	, anchor_(nullptr)
{
	top_nodes.first = top_nodes.last = nullptr;
	subregions.first = subregions.last = nullptr;
	region_subregions_list.prev = region_subregions_list.next = nullptr;

	if (parent_) {
		JIVE_LIST_PUSH_BACK(parent_->subregions, this, region_subregions_list);
		depth_ = parent_->depth() + 1;
	}

	graph->on_region_create(this);
}

void
region::reparent(jive::region * new_parent) noexcept
{
	std::function<void (jive::region *, size_t)> set_depth_recursive = [&](
		jive::region * region,
		size_t new_depth)
	{
		region->depth_ = new_depth;
		jive::region * subregion;
		JIVE_LIST_ITERATE(region->subregions, subregion, region_subregions_list)
			set_depth_recursive(subregion, new_depth + 1);
	};

	JIVE_LIST_REMOVE(parent()->subregions, this, region_subregions_list);

	parent_ = new_parent;
	size_t new_depth = new_parent->depth() + 1;
	if (new_depth != depth())
		set_depth_recursive(this, new_depth);

	JIVE_LIST_PUSH_BACK(parent_->subregions, this, region_subregions_list);
}

bool
region::contains(const jive::node * node) const noexcept
{
	const jive::region * tmp = node->region();
	while (tmp->depth() >= depth()) {
		if (tmp == this)
			return true;
		tmp = tmp->parent();
		if (!tmp)
			break;
	}
	return false;
}

void
region::copy(region * target, substitution_map & smap, bool copy_top, bool copy_bottom) const
{
	std::function<void (
		const region*,
		region*,
		std::vector<std::vector<const jive::node*>>&,
		substitution_map&,
		bool,
		bool
	)>
	pre_copy_region = [&] (
		const region * source,
		region * target,
		std::vector<std::vector<const jive::node*>> & context,
		substitution_map & smap,
		bool copy_top,
		bool copy_bottom)
	{
		for (const auto & node : source->nodes) {
			if (!copy_top && &node == source->top())
				continue;
			if (!copy_bottom && &node == source->bottom())
				continue;
	
			if (node.depth() >= context.size())
				context.resize(node.depth()+1);
			context[node.depth()].push_back(&node);
		}

		jive::region * subregion;
		JIVE_LIST_ITERATE(source->subregions, subregion, region_subregions_list) {
			jive::region * target_subregion = new jive::region(target, target->graph());
			smap.insert(subregion, target_subregion);
			pre_copy_region(subregion, target_subregion, context, smap, true, true);
		}
	};

	smap.insert(this, target);
	std::vector<std::vector<const jive::node*>> context;
	pre_copy_region(this, target, context, smap, copy_top, copy_bottom);

	for (size_t n = 0; n < context.size(); n++) {
		for (const auto node : context[n]) {
			target = smap.lookup(node->region());
			jive::node * new_node = node->copy(target, smap);
			if (node->region()->top() == node)
				target->set_top(new_node);
			if (node->region()->bottom() == node)
				target->set_bottom(new_node);
		}
	}
}

}	//namespace

#ifdef JIVE_DEBUG
void
jive_region_verify_top_node_list(struct jive::region * region)
{
	jive::region * subregion;
	JIVE_LIST_ITERATE(region->subregions, subregion, region_subregions_list)
		jive_region_verify_top_node_list(subregion);

	/* check whether all nodes in the top_node_list are really nullary nodes */
	jive::node * node;
	JIVE_LIST_ITERATE(region->top_nodes, node, region_top_node_list)
		JIVE_DEBUG_ASSERT(node->ninputs() == 0);

	/* check whether all nullary nodes from the region are in the top_node_list */
	for (const auto & node : region->nodes) {
		if (node.ninputs() != 0)
			continue;

		jive::node * top;
		JIVE_LIST_ITERATE(region->top_nodes, top, region_top_node_list) {
			if (top == &node)
				break;
		}
		if (top == NULL)
			JIVE_DEBUG_ASSERT(0);
	}
}
#endif
