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
struct jive_node;
struct jive_region;

namespace jive {
namespace detail {

template<typename T>
class traverser_iterator {
public:
	typedef std::input_iterator_tag iterator_category;
	typedef jive_node * value_type;
	typedef ssize_t difference_type;
	typedef value_type * pointer;
	typedef value_type & reference;

	constexpr
	traverser_iterator(T * traverser = nullptr, jive_node * node = nullptr) noexcept
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
	jive_node * node_;
};

}

class topdown_traverser final {
public:
	~topdown_traverser() noexcept;

	explicit
	topdown_traverser(jive_graph * graph);

	jive_node *
	next();

	typedef detail::traverser_iterator<topdown_traverser> iterator;
	typedef jive_node * value_type;
	inline iterator begin() { return iterator(this, next()); }
	inline iterator end() { return iterator(this, nullptr); }

private:
	bool
	predecessors_visited(const jive_node * node) noexcept;

	void
	check_node(jive_node * node);

	void
	init_top_nodes(jive_region * region);

	void
	node_create(jive_node * node);

	void
	input_change(input * in, output * old_origin, output * new_origin);

	jive_traversal_tracker tracker_;
	std::vector<callback> callbacks_;
};

class bottomup_traverser final {
public:
	~bottomup_traverser() noexcept;

	explicit
	bottomup_traverser(jive_graph * graph, bool revisit = false);

	jive_node *
	next();

	typedef detail::traverser_iterator<bottomup_traverser> iterator;
	typedef jive_node * value_type;
	inline iterator begin() { return iterator(this, next()); }
	inline iterator end() { return iterator(this, nullptr); }

private:
	void
	check_node(jive_node * node);

	void
	node_create(jive_node * node);

	void
	node_destroy(jive_node * node);

	void
	input_change(input * in, output * old_origin, output * new_origin);

	jive_traversal_tracker tracker_;
	std::vector<callback> callbacks_;
	jive_traversal_nodestate new_nodes_state_;
};

class upward_cone_traverser final {
public:
	~upward_cone_traverser() noexcept;

	explicit
	upward_cone_traverser(jive_node * node);

	jive_node *
	next();

	typedef detail::traverser_iterator<upward_cone_traverser> iterator;
	typedef jive_node * value_type;
	inline iterator begin() { return iterator(this, next()); }
	inline iterator end() { return iterator(this, nullptr); }

private:
	void
	check_node(jive_node * node);

	void
	node_create(jive_node * node);

	void
	node_destroy(jive_node * node);

	void
	input_change(input * input, output * old_origin, output * new_origin);

	jive_traversal_tracker tracker_;
	std::vector<callback> callbacks_;
};

class bottomup_region_traverser;
class bottomup_slave_traverser final {
public:
	~bottomup_slave_traverser() noexcept;

	bottomup_slave_traverser(
		bottomup_region_traverser * master,
		const jive_region * region);

	jive_node *
	next();

	typedef detail::traverser_iterator<bottomup_slave_traverser> iterator;
	typedef jive_node * value_type;
	inline iterator begin() { return iterator(this, next()); }
	inline iterator end() { return iterator(this, nullptr); }

private:
	bottomup_region_traverser * master_;
	const jive_region * region_;
	jive_tracker_depth_state * frontier_state_;

	detail::intrusive_hash_anchor<bottomup_slave_traverser> hash_chain_;

	typedef detail::intrusive_hash_accessor<
		const jive_region *,
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
	pass(jive_node * node);

	bottomup_slave_traverser *
	map_region(const jive_region * region);

private:
	typedef detail::owner_intrusive_hash<
		const jive_region *,
		bottomup_slave_traverser,
		bottomup_slave_traverser::hash_chain_accessor
	> slave_traverser_hash;

	void
	check_above(jive_node * node);

	jive_tracker_nodestate *
	map_node(jive_node * node);

	jive_graph * graph_;
	slave_traverser_hash region_hash_;
	jive_tracker_slot slot_; /* FIXME: RAII */
	jive_tracker_depth_state * behind_state_; /* FIXME: RAII */

	friend class bottomup_slave_traverser;
};

}

#endif
