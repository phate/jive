/*
 * Copyright 2010 2011 2012 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_SHAPED_REGION_H
#define JIVE_REGALLOC_SHAPED_REGION_H

#include <ostream>

#include <jive/common.h>
#include <jive/regalloc/shaped-node.h>
#include <jive/regalloc/xpoint.h>
#include <jive/util/intrusive-list.h>

struct jive_node;
struct jive_region;

class jive_cut;
class jive_shaped_graph;
class jive_shaped_region;
class jive_shaped_ssavar;

class jive_cut {
public:
	typedef jive::detail::intrusive_list<
		jive_shaped_node,
		jive_shaped_node::list_accessor
	> nodes_list;

	~jive_cut() noexcept;

	inline jive_cut(jive_shaped_region * shaped_region) noexcept
		: shaped_region_(shaped_region)
	{
	}

	inline jive_shaped_region & shaped_region() noexcept { return *shaped_region_; }
	inline const jive_shaped_region & shaped_region() const noexcept { return *shaped_region_; }

	/** \brief Insert node before given position in this cut */
	jive_shaped_node *
	insert(jive_shaped_node * before, jive_node * node);

	/** \brief Append node to end of this cut */
	inline jive_shaped_node *
	append(jive_node * node)
	{
		return insert(nullptr, node);
	}

	/** \brief Split cut at location
	 *
	 * Split this cut at the given location: all nodes before this point
	 * (if any) are moved into a new cut above, all nodes before this
	 * point (if any) are moved into a new cut below. Returns the
	 * (empty) cut at the split point. */
	jive_cut *
	split(jive_shaped_node * at);

	jive_shaped_region * shaped_region_;

	inline const nodes_list & nodes() const noexcept { return nodes_; }
	inline nodes_list & nodes() noexcept { return nodes_; }

private:
	/** \brief Remove all nodes from cut
	 *
	 * The nodes are returned to a state where they are not inserted
	 * anywhere in any cut. */
	void
	remove_nodes() noexcept;
	
	jive::detail::intrusive_list_anchor<jive_cut> region_cut_list_;
	typedef jive::detail::intrusive_list_accessor<
		jive_cut,
		&jive_cut::region_cut_list_
	> list_accessor;

	nodes_list nodes_;

	friend class jive_shaped_node;
	friend class jive_shaped_region;
};

class jive_shaped_region {
public:
	typedef jive::detail::owner_intrusive_list<jive_cut, jive_cut::list_accessor> cut_list;

	~jive_shaped_region() noexcept;

	inline jive_shaped_region(
		jive_shaped_graph * shaped_graph,
		jive_region * region) noexcept
		: shaped_graph_(shaped_graph)
		, region_(region)
	{
	}

	inline jive_shaped_graph & shaped_graph() noexcept { return *shaped_graph_; }
	inline const jive_shaped_graph & shaped_graph() const noexcept { return *shaped_graph_; }
	inline jive_region * region() const noexcept { return region_; }

	inline cut_list & cuts() noexcept { return cuts_; }
	inline const cut_list & cuts() const noexcept { return cuts_; }

	inline const jive_varcut & active_top() const noexcept { return active_top_; }

	/** \brief Create a new top-most cut */
	inline jive_cut *
	create_top_cut()
	{
		return create_cut(cuts_.begin());
	}

	jive_shaped_node *
	first_in_region() noexcept;

	jive_shaped_node *
	last_in_region() noexcept;

private:
	void
	add_active_top(jive_shaped_ssavar * shaped_ssavar, size_t count);

	void
	remove_active_top(jive_shaped_ssavar * shaped_ssavar, size_t count) noexcept;

	/** \brief Clear all cuts
	 *
	 * Clears all cuts for this region. All nodes are removed. */
	void
	clear_cuts() noexcept;

	jive_cut *
	create_cut(cut_list::iterator before);

	void
	debug_stream(const std::string& indent, std::ostream& os) const;

	jive_shaped_graph * shaped_graph_;
	jive_region * region_;
	cut_list cuts_;
	jive_varcut active_top_;
	jive::detail::intrusive_hash_anchor<jive_shaped_region> hash_chain_;

	typedef jive::detail::intrusive_hash_accessor <
		jive_region *,
		jive_shaped_region,
		&jive_shaped_region::region_,
		&jive_shaped_region::hash_chain_
	> hash_chain_accessor;

	friend class jive_cut;
	friend class jive_shaped_graph;
	friend class jive_shaped_node;
	friend class jive_shaped_ssavar;
};

#endif
