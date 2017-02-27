/*
 * Copyright 2010 2011 2012 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_TRAVERSER_H
#define JIVE_VSDG_TRAVERSER_H

#include <stdbool.h>
#include <stdlib.h>

struct jive_graph;

namespace jive {
namespace detail {

template<typename T>
class traverser_iterator {
public:
	typedef std::input_iterator_tag iterator_category;
	typedef jive::node * value_type;
	typedef ssize_t difference_type;
	typedef value_type * pointer;
	typedef value_type & reference;

	constexpr
	traverser_iterator(T * traverser = nullptr, jive::node * node = nullptr) noexcept
		: traverser_(traverser)
		, node_(node)
	{
	}

	inline const traverser_iterator &
	operator++() noexcept
	{
		node_ = traverser_->next();
		return *this;
	}

	inline bool
	operator==(const traverser_iterator& other) const noexcept
	{
		return traverser_ == other.traverser_ && node_ == other.node_;
	}

	inline bool
	operator!=(const traverser_iterator& other) const noexcept
	{
		return !(*this == other);
	}

	inline value_type & operator*() noexcept { return node_; }

	inline value_type * operator->() noexcept { return *node_; }

private:
	T * traverser_;
	jive::node * node_;
};

}

enum class traversal_nodestate {
	ahead = -1,
	frontier = 0,
	behind = +1
};

/* support class to track traversal states of nodes */
class traversal_tracker final {
public:
	inline
	traversal_tracker(jive_graph * graph);
	
	inline traversal_nodestate
	get_nodestate(jive::node * node);
	
	inline void
	set_nodestate(jive::node * node, traversal_nodestate state);
	
	inline jive::node *
	peek_top();
	
	inline jive::node *
	peek_bottom();

private:
	tracker tracker_;
};

class topdown_traverser final {
public:
	~topdown_traverser() noexcept;

	explicit
	topdown_traverser(jive::region * region);

	jive::node *
	next();

	inline jive::region *
	region() const noexcept
	{
		return region_;
	}

	typedef detail::traverser_iterator<topdown_traverser> iterator;
	typedef jive::node * value_type;
	inline iterator begin() { return iterator(this, next()); }
	inline iterator end() { return iterator(this, nullptr); }

private:
	bool
	predecessors_visited(const jive::node * node) noexcept;

	void
	node_create(jive::node * node);

	void
	iport_change(iport * in, oport * old_origin, oport * new_origin);

	jive::region * region_;
	traversal_tracker tracker_;
	std::vector<callback> callbacks_;
};

class bottomup_traverser final {
public:
	~bottomup_traverser() noexcept;

	explicit
	bottomup_traverser(jive_graph * graph, bool revisit = false);

	jive::node *
	next();

	typedef detail::traverser_iterator<bottomup_traverser> iterator;
	typedef jive::node * value_type;
	inline iterator begin() { return iterator(this, next()); }
	inline iterator end() { return iterator(this, nullptr); }

private:
	void
	check_node(jive::node * node);

	void
	node_create(jive::node * node);

	void
	node_destroy(jive::node * node);

	void
	iport_change(iport * in, oport * old_origin, oport * new_origin);

	traversal_tracker tracker_;
	std::vector<callback> callbacks_;
	traversal_nodestate new_nodes_state_;
};

class upward_cone_traverser final {
public:
	~upward_cone_traverser() noexcept;

	explicit
	upward_cone_traverser(jive::node * node);

	jive::node *
	next();

	typedef detail::traverser_iterator<upward_cone_traverser> iterator;
	typedef jive::node * value_type;
	inline iterator begin() { return iterator(this, next()); }
	inline iterator end() { return iterator(this, nullptr); }

private:
	void
	check_node(jive::node * node);

	void
	node_create(jive::node * node);

	void
	node_destroy(jive::node * node);

	void
	iport_change(iport * input, oport * old_origin, oport * new_origin);

	traversal_tracker tracker_;
	std::vector<callback> callbacks_;
};

class bottomup_region_traverser;
class bottomup_slave_traverser final {
public:
	~bottomup_slave_traverser() noexcept;

	bottomup_slave_traverser(
		bottomup_region_traverser * master,
		const jive::region * region);

	jive::node *
	next();

	typedef detail::traverser_iterator<bottomup_slave_traverser> iterator;
	typedef jive::node * value_type;
	inline iterator begin() { return iterator(this, next()); }
	inline iterator end() { return iterator(this, nullptr); }

private:
	bottomup_region_traverser * master_;
	const jive::region * region_;
	jive_tracker_depth_state * frontier_state_;

	detail::intrusive_hash_anchor<bottomup_slave_traverser> hash_chain_;

	typedef detail::intrusive_hash_accessor<
		const jive::region *,
		bottomup_slave_traverser,
		&bottomup_slave_traverser::region_,
		&bottomup_slave_traverser::hash_chain_
	> hash_chain_accessor;

	friend class bottomup_region_traverser;
};

class bottomup_region_traverser {
public:
	~bottomup_region_traverser() noexcept;

	explicit
	bottomup_region_traverser(jive_graph * graph);

	void
	pass(jive::node * node);

	bottomup_slave_traverser *
	map_region(const jive::region * region);

private:
	typedef detail::owner_intrusive_hash<
		const jive::region *,
		bottomup_slave_traverser,
		bottomup_slave_traverser::hash_chain_accessor
	> slave_traverser_hash;

	void
	check_above(jive::node * node);

	jive_tracker_nodestate *
	map_node(jive::node * node);

	jive_graph * graph_;
	slave_traverser_hash region_hash_;
	jive_tracker_slot slot_; /* FIXME: RAII */
	jive_tracker_depth_state * behind_state_; /* FIXME: RAII */

	friend class bottomup_slave_traverser;
};

/* traversal tracker implementation */

traversal_tracker::traversal_tracker(jive_graph * graph)
	: tracker_(graph, 2)
{
}

traversal_nodestate
traversal_tracker::get_nodestate(jive::node * node)
{
	return static_cast<traversal_nodestate>(tracker_.get_nodestate(node));
}

void
traversal_tracker::set_nodestate(
	jive::node * node,
	traversal_nodestate state)
{
	tracker_.set_nodestate(node, static_cast<size_t>(state));
}

jive::node *
traversal_tracker::peek_top()
{
	return tracker_.peek_top(static_cast<size_t>(traversal_nodestate::frontier));
}

jive::node *
traversal_tracker::peek_bottom()
{
	return tracker_.peek_bottom(static_cast<size_t>(traversal_nodestate::frontier));
}

}

#endif
