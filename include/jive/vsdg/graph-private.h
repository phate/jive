/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_GRAPH_PRIVATE_H
#define JIVE_VSDG_GRAPH_PRIVATE_H

#include <jive/common.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

struct jive_tracker_slot_reservation {
	jive_tracker_slot slot;
	bool in_use;
};

struct jive_tracker_nodestate_list {
	jive_tracker_nodestate * first;
	jive_tracker_nodestate * last;
};

struct jive_tracker_depth_state {
	size_t top_occupied;
	size_t bottom_occupied;
	size_t count;
	size_t space;
	std::vector<jive_tracker_nodestate_list> nodestates_per_depth;
	
	struct {
		jive_tracker_depth_state * prev;
		jive_tracker_depth_state * next;
	} graph_cached_tracker_states;
};

jive_tracker_slot
jive_graph_reserve_tracker_slot_slow(jive_graph * self);

static inline jive_tracker_slot
jive_graph_reserve_tracker_slot(jive_graph * self)
{
	size_t n;
	for (n = 0; n < self->tracker_slots.size(); n++) {
		if (!self->tracker_slots[n].in_use) {
			/* in theory, overflow might be possible, causing
			a cookie to be reused... just catch this case
			even if it is never going to happen in real life */
			if (self->tracker_slots[n].slot.cookie == (size_t) -1)
				continue;
			self->tracker_slots[n].slot.cookie ++;
			self->tracker_slots[n].in_use = true;
			return self->tracker_slots[n].slot;
		}
	}
	
	return jive_graph_reserve_tracker_slot_slow(self);
}

static inline void
jive_graph_return_tracker_slot(jive_graph * self, jive_tracker_slot slot)
{
	JIVE_DEBUG_ASSERT(self->tracker_slots[slot.index].in_use);
	self->tracker_slots[slot.index].in_use = false;
}

static inline jive_tracker_depth_state *
jive_graph_reserve_tracker_depth_state(jive_graph * self)
{
	jive_tracker_depth_state * state = new jive_tracker_depth_state;
	state->count = 0;
	state->space = 0;
	return state;
}

static inline void
jive_graph_return_tracker_depth_state(jive_graph * self, jive_tracker_depth_state * state)
{
	delete state;
}

#endif
