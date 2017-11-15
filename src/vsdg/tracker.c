/*
 * Copyright 2010 2011 2012 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/common.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/simple_node.h>
#include <jive/vsdg/tracker.h>

using namespace std::placeholders;

struct jive_tracker_nodestate_list {
	jive::tracker_nodestate * first;
	jive::tracker_nodestate * last;
};

namespace jive {

/* tracker depth state */

class tracker_depth_state {
public:
	inline
	tracker_depth_state()
	: count_(0)
	, top_depth_(0)
	, bottom_depth_(0)
	{}

	tracker_depth_state(const tracker_depth_state&) = delete;

	tracker_depth_state(tracker_depth_state&&) = delete;

	tracker_depth_state &
	operator=(const tracker_depth_state&) = delete;

	tracker_depth_state &
	operator=(tracker_depth_state&&) = delete;

	inline tracker_nodestate *
	peek_top() const noexcept
	{
		return count_ ? nodestates_[top_depth_].first : nullptr;
	}

	inline tracker_nodestate *
	peek_bottom() const noexcept
	{
		return count_ ? nodestates_[bottom_depth_].first : nullptr;
	}

	inline void
	add(tracker_nodestate * nodestate, size_t depth)
	{
		size_t old_size = nodestates_.size();
		if (depth >= old_size) {
			size_t new_size = old_size * 2 + 1;
			if (new_size <= depth) new_size = depth + 1;

			nodestates_.resize(new_size);
			for (size_t n = old_size; n < new_size; n++)
				nodestates_[n].first = nodestates_[n].last = nullptr;
		}

		JIVE_LIST_PUSH_BACK(nodestates_[depth], nodestate, state_node_list);

		count_++;
		if (count_ == 1) {
			top_depth_= depth;
			bottom_depth_= depth;
		} else {
			if (depth < top_depth_)
				top_depth_ = depth;
			if (depth > bottom_depth_)
				bottom_depth_ = depth;
		}
	}

	inline void
	remove(tracker_nodestate * nodestate, size_t depth)
	{
		JIVE_LIST_REMOVE(nodestates_[depth], nodestate, state_node_list);

		count_--;
		if (count_ == 0)
			return;

		if (depth == top_depth_) {
			while (!nodestates_[top_depth_].first)
				top_depth_++;
		}

		if (depth == bottom_depth_) {
			while (!nodestates_[bottom_depth_].first)
				bottom_depth_--;
		}

		JIVE_DEBUG_ASSERT(top_depth_ <= bottom_depth_);
	}

	inline tracker_nodestate *
	pop_top()
	{
		auto nodestate = peek_top();
		if (nodestate) {
			remove(nodestate, top_depth_);
			nodestate->state = jive_tracker_nodestate_none;
		}

		return nodestate;
	}

	inline tracker_nodestate *
	pop_bottom()
	{
		auto nodestate = peek_bottom();
		if (nodestate) {
			remove(nodestate, bottom_depth_);
			nodestate->state = jive_tracker_nodestate_none;
		}

		return nodestate;
	}

private:
	size_t count_;
	size_t top_depth_;
	size_t bottom_depth_;
	std::vector<jive_tracker_nodestate_list> nodestates_;
};

/* tracker */

tracker::~tracker()
{
	graph_->destroy_tracker_slot(slot_);
}

tracker::tracker(jive::graph * graph, size_t nstates)
	: graph_(graph)
	, slot_(graph->create_tracker_slot())
	, states_(nstates)
{
	for (size_t n = 0; n < states_.size(); n++)
		states_[n]= std::make_unique<tracker_depth_state>();

	depth_callback_ = graph->on_node_depth_change.connect(
		std::bind(&tracker::node_depth_change, this, _1, _2));
	destroy_callback_ = graph->on_node_destroy.connect(
		std::bind(&tracker::node_destroy, this, _1));
}

tracker_nodestate *
tracker::map_node(jive::node * node)
{
	return node->tracker_state(slot_);
}

void
tracker::node_depth_change(jive::node * node, size_t old_depth)
{
	auto nodestate = map_node(node);
	if (nodestate->state >= states_.size()) {
		return;
	}
	states_[nodestate->state]->remove(nodestate, old_depth);
	states_[nodestate->state]->add(nodestate, node->depth());
}

void
tracker::node_destroy(jive::node * node)
{
	auto nodestate = map_node(node);
	if (nodestate->state >= states_.size()) {
		return;
	}
	states_[nodestate->state]->remove(nodestate, node->depth());
}

ssize_t
tracker::get_nodestate(jive::node * node)
{
	return map_node(node)->state;
}

void
tracker::set_nodestate(jive::node * node, size_t state)
{
	auto nodestate = map_node(node);
	if (nodestate->state != state) {
		if (nodestate->state < states_.size())
			states_[nodestate->state]->remove(nodestate, node->depth());

		nodestate->state = state;
		if (nodestate->state < states_.size())
			states_[nodestate->state]->add(nodestate, node->depth());
	}
}

jive::node *
tracker::peek_top(size_t state) const
{
	auto nodestate = states_[state]->pop_top();
	return nodestate ? nodestate->node() : nullptr;
}

jive::node *
tracker::peek_bottom(size_t state) const
{
	auto nodestate = states_[state]->pop_bottom();
	return nodestate ? nodestate->node() : nullptr;
}

computation_tracker::computation_tracker(jive::graph * graph)
	: graph_(graph)
	, slot_(graph->create_tracker_slot())
	, nodestates_(std::make_unique<tracker_depth_state>())
{
}

computation_tracker::~computation_tracker() noexcept
{
	graph_->destroy_tracker_slot(slot_);
}

tracker_nodestate *
computation_tracker::map_node(jive::node * node)
{
	return node->tracker_state(slot_);
}

void
computation_tracker::invalidate(jive::node * node)
{
	auto nodestate = map_node(node);
	if (nodestate->state == jive_tracker_nodestate_none) {
		nodestates_->add(nodestate, node->depth());
		nodestate->state = 0;
	}
}

void
computation_tracker::invalidate_below(jive::node * node)
{
	for (size_t n = 0; n < node->noutputs(); n++) {
		for (const auto & user : *node->output(n))
			invalidate(user->node());
	}
}

jive::node *
computation_tracker::pop_top()
{
	auto nodestate = nodestates_->pop_top();
	return nodestate ? nodestate->node() : nullptr;
}

}
