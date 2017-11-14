/*
 * Copyright 2010 2011 2012 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_TRACKER_H
#define JIVE_VSDG_TRACKER_H

#include <stdbool.h>
#include <stddef.h>

#include <jive/util/callbacks.h>

struct jive_notifier;

static const size_t jive_tracker_nodestate_none = (size_t) -1;

struct jive_tracker_slot {
	size_t index;
	size_t cookie;
};

namespace jive {

class graph;
class node;
class region;
class tracker_nodestate;
class tracker_depth_state;

/* Track states of nodes within the graph. Each node can logically be in
 * one of the numbered states, plus another "initial" state. All nodes are
 * at the beginning assumed to be implicitly in this "initial" state. */
struct tracker {
public:
	~tracker() noexcept;
	
	tracker(jive::graph * graph, size_t nstates);

	/* get state of the node */
	ssize_t
	get_nodestate(jive::node * node);

	/* set state of the node */
	void
	set_nodestate(jive::node * node, size_t state);

	/* get one of the top nodes for the given state */
	jive::node *
	peek_top(size_t state) const;

	/* get one of the bottom nodes for the given state */
	jive::node *
	peek_bottom(size_t state) const;

private:
	void
	node_depth_change(jive::node * node, size_t old_depth);

	void
	node_destroy(jive::node * node);

	tracker_nodestate *
	map_node(jive::node * node);

	jive::graph * graph_;
	/* FIXME: need RAII idiom for slot reservation */
	jive_tracker_slot slot_;
	/* FIXME: need RAII idiom for state reservation */
	std::vector<std::unique_ptr<tracker_depth_state>> states_;

	callback depth_callback_, destroy_callback_;
};

class computation_tracker {
public:
	computation_tracker(jive::graph * graph);
	
	~computation_tracker() noexcept;
	
	void
	invalidate(jive::node * node);
	
	void
	invalidate_below(jive::node * node);
	
	jive::node *
	pop_top();

private:
	tracker_nodestate *
	map_node(jive::node * node);

	jive::graph * graph_;
	/* FIXME: need RAII idiom for slot reservation */
	jive_tracker_slot slot_;
	/* FIXME: need RAII idiom for state reservation */
	std::unique_ptr<tracker_depth_state> nodestates_;
};

}

#endif
