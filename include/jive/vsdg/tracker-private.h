/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_TRACKER_PRIVATE_H
#define JIVE_VSDG_TRACKER_PRIVATE_H

#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/tracker.h>

static inline void
jive_tracker_depth_state_add(jive_tracker_depth_state * self, jive_tracker_nodestate * nodestate,
	size_t depth)
{
	if (depth >= self->space) {
		size_t new_space = self->space * 2 + 1;
		if (new_space <= depth)
			new_space = depth + 1;
		self->nodestates_per_depth.resize(new_space);
		for (size_t n = self->space; n < new_space; n++) {
			self->nodestates_per_depth[n].first = self->nodestates_per_depth[n].last = 0;
		}
		self->space = new_space;
	}
	
	JIVE_LIST_PUSH_BACK(self->nodestates_per_depth[depth], nodestate, state_node_list);
	
	self->count ++;
	if (self->count == 1) {
		self->top_occupied = depth;
		self->bottom_occupied = depth;
	} else {
		if (depth < self->top_occupied)
			self->top_occupied = depth;
		if (depth > self->bottom_occupied)
			self->bottom_occupied = depth;
	}
}

static inline void
jive_tracker_depth_state_remove(jive_tracker_depth_state * self, jive_tracker_nodestate * nodestate,
	size_t depth)
{
	JIVE_LIST_REMOVE(self->nodestates_per_depth[depth], nodestate, state_node_list);
	
	self->count --;
	if (self->count == 0)
		return;
	
	if (depth == self->top_occupied) {
		while (!self->nodestates_per_depth[self->top_occupied].first)
			self->top_occupied ++;
	}
	if (depth == self->bottom_occupied) {
		while (!self->nodestates_per_depth[self->bottom_occupied].first)
			self->bottom_occupied --;
	}
	
	JIVE_DEBUG_ASSERT(self->top_occupied <= self->bottom_occupied);
}

static inline jive_tracker_nodestate *
jive_tracker_depth_state_peek_top(const jive_tracker_depth_state * self)
{
	if (self->count)
		return self->nodestates_per_depth[self->top_occupied].first;
	else
		return 0;
}

static inline jive_tracker_nodestate *
jive_tracker_depth_state_peek_bottom(const jive_tracker_depth_state * self)
{
	if (self->count)
		return self->nodestates_per_depth[self->bottom_occupied].first;
	else
		return 0;
}

static inline jive_tracker_nodestate *
jive_tracker_depth_state_pop_top(jive_tracker_depth_state * self)
{
	jive_tracker_nodestate * nodestate = jive_tracker_depth_state_peek_top(self);
	if (nodestate) {
		jive_tracker_depth_state_remove(self, nodestate, self->top_occupied);
		nodestate->state = jive_tracker_nodestate_none;
	}
	return nodestate;
}

static inline jive_tracker_nodestate *
jive_tracker_depth_state_pop_bottom(jive_tracker_depth_state * self)
{
	jive_tracker_nodestate * nodestate = jive_tracker_depth_state_peek_bottom(self);
	if (nodestate) {
		jive_tracker_depth_state_remove(self, nodestate, self->bottom_occupied);
		nodestate->state = jive_tracker_nodestate_none;
	}
	return nodestate;
}

#endif
