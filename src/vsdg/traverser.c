/*
 * Copyright 2010 2011 2012 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/common.h>
#include <jive/util/list.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/simple_node.h>
#include <jive/vsdg/tracker-private.h>
#include <jive/vsdg/tracker.h>
#include <jive/vsdg/traverser.h>

#include <string.h>

using namespace std::placeholders;

/* top down traverser */

namespace jive {

topdown_traverser::~topdown_traverser() noexcept {}

topdown_traverser::topdown_traverser(jive::region * region)
	: region_(region)
	, tracker_(region->graph())
{
	jive::node * node;
	JIVE_LIST_ITERATE(region->top_nodes, node, region_top_node_list) {
		tracker_.set_nodestate(node, traversal_nodestate::frontier);
	}

	for (size_t n = 0; n < region->narguments(); n++) {
		auto argument = region->argument(n);
		for (const auto & user : argument->users) {
			if (!user->node() || !predecessors_visited(user->node()))
				continue;

			tracker_.set_nodestate(user->node(), traversal_nodestate::frontier);
		}
	}

	callbacks_.push_back(region->graph()->on_node_create.connect(
		std::bind(&topdown_traverser::node_create, this, _1)));
	callbacks_.push_back(region->graph()->on_iport_change.connect(
		std::bind(&topdown_traverser::iport_change, this, _1, _2, _3)));
}

bool
topdown_traverser::predecessors_visited(const jive::node * node) noexcept
{
	for (size_t n = 0; n < node->ninputs(); n++) {
		auto predecessor = node->input(n)->origin()->node();
		if (!predecessor)
			continue;

		if (tracker_.get_nodestate(predecessor) != traversal_nodestate::behind)
			return false;
	}

	return true;
}

jive::node *
topdown_traverser::next()
{
	jive::node * node = tracker_.peek_top();
	if (!node) return nullptr;

	tracker_.set_nodestate(node, traversal_nodestate::behind);
	for (size_t n = 0; n < node->noutputs(); n++) {
		for (const auto & user : node->output(n)->users) {
			if (!user->node())
				continue;

			if (tracker_.get_nodestate(user->node()) == traversal_nodestate::ahead)
				tracker_.set_nodestate(user->node(), traversal_nodestate::frontier);
		}
	}

	return node;
}

void
topdown_traverser::node_create(jive::node * node)
{
	if (node->region() != region())
		return;

	if (predecessors_visited(node))
		tracker_.set_nodestate(node, traversal_nodestate::behind);
	else
		tracker_.set_nodestate(node, traversal_nodestate::frontier);
}

void
topdown_traverser::iport_change(iport * in, oport * old_origin, oport * new_origin)
{
	if (in->region() != region() || !in->node())
		return;

	auto state = tracker_.get_nodestate(in->node());
	
	/* ignore nodes that have been traversed already, or that are already
	marked for later traversal */
	if (state != traversal_nodestate::ahead)
		return;
	
	/* make sure node is visited eventually, might now be visited earlier
	as depth of the node could be lowered */
	tracker_.set_nodestate(in->node(), traversal_nodestate::frontier);
}

/* bottom up traverser */

bottomup_traverser::~bottomup_traverser() noexcept {}

bottomup_traverser::bottomup_traverser(jive::region * region, bool revisit)
	: region_(region)
	, tracker_(region->graph())
	, new_node_state_(revisit ? traversal_nodestate::frontier : traversal_nodestate::behind)
{
	jive::node * node;
	JIVE_LIST_ITERATE(region->bottom_nodes, node, region_bottom_list)
		tracker_.set_nodestate(node, traversal_nodestate::frontier);

	for (size_t n = 0; n < region->nresults(); n++) {
		auto node = region->result(n)->origin()->node();
		if (node && !node->has_successors())
			tracker_.set_nodestate(node, traversal_nodestate::frontier);
	}

	callbacks_.push_back(region->graph()->on_node_create.connect(
		std::bind(&bottomup_traverser::node_create, this, _1)));
	callbacks_.push_back(region->graph()->on_node_destroy.connect(
		std::bind(&bottomup_traverser::node_destroy, this, _1)));
	callbacks_.push_back(region->graph()->on_iport_change.connect(
		std::bind(&bottomup_traverser::iport_change, this, _1, _2, _3)));
}

jive::node *
bottomup_traverser::next()
{
	auto node = tracker_.peek_bottom();
	if (!node) return nullptr;

	tracker_.set_nodestate(node, traversal_nodestate::behind);
	for (size_t n = 0; n < node->ninputs(); n++) {
		auto producer = node->input(n)->origin()->node();
		if (producer && tracker_.get_nodestate(producer) == traversal_nodestate::ahead)
			tracker_.set_nodestate(producer, traversal_nodestate::frontier);
	}
	return node;
}

void
bottomup_traverser::node_create(jive::node * node)
{
	if (node->region() != region())
		return;

	tracker_.set_nodestate(node, new_node_state_);
}

void
bottomup_traverser::node_destroy(jive::node * node)
{
	if (node->region() != region())
		return;

	for (size_t n = 0; n < node->ninputs(); n++) {
		auto producer = node->input(n)->origin()->node();
		if (producer && tracker_.get_nodestate(producer) == traversal_nodestate::ahead)
			tracker_.set_nodestate(producer, traversal_nodestate::frontier);
	}
}

void
bottomup_traverser::iport_change(iport * in, oport * old_origin, oport * new_origin)
{
	if (in->region() != region() || !in->node())
		return;

	traversal_nodestate state = tracker_.get_nodestate(old_origin->node());
	
	/* ignore nodes that have been traversed already, or that are already
	marked for later traversal */
	if (state != traversal_nodestate::ahead)
		return;
	
	/* make sure node is visited eventually, might now be visited earlier
	as there (potentially) is one less obstructing node below */
	tracker_.set_nodestate(old_origin->node(), traversal_nodestate::frontier);
}

/* upward cone traverser */

upward_cone_traverser::~upward_cone_traverser() noexcept {}

upward_cone_traverser::upward_cone_traverser(jive::node * node)
	: tracker_(node->graph())
{
	jive_graph * graph = node->graph();
	
	tracker_.set_nodestate(node, traversal_nodestate::frontier);
	
	callbacks_.push_back(graph->on_node_destroy.connect(
		std::bind(&upward_cone_traverser::node_destroy, this, _1)));
	callbacks_.push_back(graph->on_iport_change.connect(
		std::bind(&upward_cone_traverser::iport_change, this, _1, _2, _3)));
}

void
upward_cone_traverser::check_node(jive::node * node)
{
	if (tracker_.get_nodestate(node) == traversal_nodestate::ahead) {
		tracker_.set_nodestate(node, traversal_nodestate::frontier);
	}
}


void
upward_cone_traverser::node_destroy(jive::node * node)
{
	traversal_nodestate state = tracker_.get_nodestate(node);
	
	if (state != traversal_nodestate::frontier) {
		return;
	}
	
	for (size_t n = 0; n < node->ninputs(); n++) {
		check_node(dynamic_cast<jive::output*>(node->input(n)->origin())->node());
	}
}

void
upward_cone_traverser::iport_change(iport * in, oport * old_origin, oport * new_origin)
{
	if (!in->node())
		return;

	auto output = dynamic_cast<jive::output*>(old_origin);

	/* for node of new origin, it may now belong to the cone */
	traversal_nodestate state = tracker_.get_nodestate(in->node());
	if (state != traversal_nodestate::ahead) {
		state = tracker_.get_nodestate(output->node());
		if (state == traversal_nodestate::ahead)
			tracker_.set_nodestate(output->node(), traversal_nodestate::frontier);
	}
	
	/* for node of old origin, it may cease to belong to the cone */
	state = tracker_.get_nodestate(output->node());
	if (state == traversal_nodestate::frontier) {
		size_t n;
		for (n = 0; n < output->node()->noutputs(); n++) {
			jive::output * out = dynamic_cast<jive::output*>(output->node()->output(n));
			for (auto user : out->users) {
				if (user == in)
					continue;
				auto input = dynamic_cast<jive::input*>(user);
				state = tracker_.get_nodestate(input->node());
				if (state != traversal_nodestate::ahead)
					return;
			}
		}
		tracker_.set_nodestate(output->node(), traversal_nodestate::ahead);
	}
}

jive::node *
upward_cone_traverser::next()
{
	jive::node * node = tracker_.peek_bottom();
	if (!node) {
		return nullptr;
	}
	tracker_.set_nodestate(node, traversal_nodestate::behind);
	for (size_t n = 0; n < node->ninputs(); n++) {
		check_node(dynamic_cast<jive::output*>(node->input(n)->origin())->node());
	}
	return node;
}

/* bottom up slave traverser */

bottomup_slave_traverser::~bottomup_slave_traverser() noexcept
{
	jive_graph_return_tracker_depth_state(master_->graph_, frontier_state_);
}

bottomup_slave_traverser::bottomup_slave_traverser(
	bottomup_region_traverser * master,
	const jive::region * region)
	: master_(master)
	, region_(region)
	, frontier_state_(jive_graph_reserve_tracker_depth_state(master_->graph_))
{
}

jive::node *
bottomup_slave_traverser::next()
{
	jive_tracker_nodestate * nodestate = jive_tracker_depth_state_pop_bottom(frontier_state_);
	if (!nodestate) {
		return nullptr;
	}
	
	jive::node * node = nodestate->node;
	
	nodestate->state = 1;
	jive_tracker_depth_state_add(master_->behind_state_, nodestate, node->depth());
	master_->check_above(node);
	
	return nodestate->node;
}

/* bottom up region traverser */

bottomup_slave_traverser *
bottomup_region_traverser::map_region(const jive::region * region)
{
	auto i = region_hash_.find(region);
	if (i != region_hash_.end()) {
		return i.ptr();
	} else {
		std::unique_ptr<bottomup_slave_traverser> trav(
			new bottomup_slave_traverser(this, region));
		
		return region_hash_.insert(std::move(trav)).ptr();
	}
}

jive_tracker_nodestate *
bottomup_region_traverser::map_node(jive::node * node)
{
	return jive_node_get_tracker_state(node, slot_);
}

void
bottomup_region_traverser::check_above(jive::node * node)
{
	for (size_t n = 0; n < node->ninputs(); n++) {
		jive::node * above = dynamic_cast<jive::output*>(node->input(n)->origin())->node();
		jive_tracker_nodestate * nodestate = map_node(above);
		if (nodestate->state != jive_tracker_nodestate_none) {
			continue;
		}
		
		jive::region * region = above->region();
		
		bottomup_slave_traverser * slave = map_region(region);
		nodestate->state = 0;
		jive_tracker_depth_state_add(slave->frontier_state_, nodestate, above->depth());
	}
}

void
bottomup_region_traverser::pass(jive::node * node)
{
	jive_tracker_nodestate * nodestate = map_node(node);
	JIVE_DEBUG_ASSERT(nodestate->state == 0);
	
	bottomup_slave_traverser * slave = map_region(node->region());
	
	jive_tracker_depth_state_remove(slave->frontier_state_, nodestate, node->depth());
	nodestate->state = 1;
	jive_tracker_depth_state_add(behind_state_, nodestate, node->depth());
	
	check_above(node);
}

bottomup_region_traverser::bottomup_region_traverser(jive_graph * graph)
	: graph_(graph)
	, slot_(jive_graph_reserve_tracker_slot(graph_))
	, behind_state_(jive_graph_reserve_tracker_depth_state(graph_))
{
	/* seed bottom nodes in root region */
	bottomup_slave_traverser * root_slave = map_region(graph_->root());
	jive::node * node;
	JIVE_LIST_ITERATE(graph_->root()->bottom_nodes, node, region_bottom_list) {
		if (node->region() != graph->root()) {
			continue;
		}
		
		jive_tracker_nodestate * nodestate = map_node(node);
		nodestate->state = 0;
		jive_tracker_depth_state_add(root_slave->frontier_state_, nodestate, node->depth());
	}
}

bottomup_region_traverser::~bottomup_region_traverser() noexcept
{
	jive_graph_return_tracker_slot(graph_, slot_);
	jive_graph_return_tracker_depth_state(graph_, behind_state_);
}

}
