/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_GRAPH_PRIVATE_H
#define JIVE_VSDG_GRAPH_PRIVATE_H

#include <jive/common.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/type.h>

struct jive_tracker_nodestate_list {
	jive::tracker_nodestate * first;
	jive::tracker_nodestate * last;
};

namespace jive {

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

}

#endif
