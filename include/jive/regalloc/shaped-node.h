/*
 * Copyright 2010 2011 2012 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_SHAPED_NODE_H
#define JIVE_REGALLOC_SHAPED_NODE_H

#include <jive/common.h>

#include <jive/regalloc/xpoint.h>
#include <jive/util/intrusive-hash.h>
#include <jive/util/intrusive-list.h>
#include <jive/vsdg/resource.h>

class jive_cut;
class jive_node;
class jive_shaped_graph;
class jive_shaped_region;
class jive_shaped_node;
class jive_shaped_ssavar;
class jive_shaped_variable;

class jive_shaped_node_downward_iterator {
public:
	typedef jive_shaped_node value_type;
	typedef value_type * pointer;
	typedef value_type & reference;
	typedef std::input_iterator_tag iterator_category;
	typedef size_t size_type;
	typedef ssize_t difference_type;

	inline
	jive_shaped_node_downward_iterator() noexcept {}

	inline explicit
	jive_shaped_node_downward_iterator(jive_shaped_node * start);

	const jive_shaped_node_downward_iterator &
	operator++();

	inline bool
	operator==(const jive_shaped_node_downward_iterator & other) const noexcept
	{
		return visit_stack_ == other.visit_stack_;
	}

	inline bool
	operator!=(const jive_shaped_node_downward_iterator & other) const noexcept
	{
		return !(*this == other);
	}

	inline reference
	operator*() const noexcept
	{
		return *visit_stack_.back();
	}

	inline pointer
	operator->() const noexcept
	{
		return visit_stack_.back();
	}

private:
	std::vector<jive_shaped_node *> visit_stack_;
	jive_shaped_graph * shaped_graph_;
};

/* represent the range of shaped nodes from one starting point to the end
 * of the shaped region, including the nodes of all subregions within */
class jive_shaped_node_to_end_range {
public:
	inline constexpr explicit
	jive_shaped_node_to_end_range(jive_shaped_node * start) noexcept
		: start_(start)
	{
	}

	typedef jive_shaped_node_downward_iterator iterator;
	typedef jive_shaped_node_downward_iterator const_iterator;
	typedef jive_shaped_node value_type;
	typedef size_t size_type;

	inline iterator
	begin() const
	{
		return jive_shaped_node_downward_iterator(start_);
	}

	inline iterator
	end() const
	{
		return jive_shaped_node_downward_iterator();
	}

private:
	jive_shaped_node * start_;
};

class jive_shaped_node {
public:
	typedef jive::detail::intrusive_hash<
		const jive_shaped_ssavar *,
		jive_nodevar_xpoint,
		jive_nodevar_xpoint::ssavar_hash_accessor
	> jive_nodevar_xpoint_hash_byssavar;

	~jive_shaped_node() noexcept;

	/* create shaped node (without inserting it anywhere) */
	inline jive_shaped_node(
		jive_shaped_graph * shaped_graph,
		jive_node * node) noexcept
		: cut_(nullptr)
		, node_(node)
		, shaped_graph_(shaped_graph)
	{
	}

	/* accessors */

	inline jive_cut * cut() const noexcept { return cut_; }
	inline jive_node * node() const noexcept { return node_; }
	inline jive_shaped_graph & shaped_graph() const noexcept { return *shaped_graph_; }

	/* previous/next node in same cut */
	jive_shaped_node * prev_in_cut() noexcept;
	jive_shaped_node * next_in_cut() noexcept;

	/* previous/next node in same region */
	jive_shaped_node * prev_in_region() noexcept;
	jive_shaped_node * next_in_region() noexcept;

	/* variables active before/after this node */
	jive_varcut get_active_before() const;
	jive_varcut get_active_after() const;

	/* get the iterable range of shaped nodes to end of region */
	inline jive_shaped_node_to_end_range
	range_to_end()
	{
		return jive_shaped_node_to_end_range(this);
	}

	bool
	is_resource_name_active_after(
		const jive_resource_name * name) const noexcept;

	bool
	is_resource_name_active_before(
		const jive_resource_name * name) const noexcept;

	inline const jive_resource_class_count &
	use_count_before() const noexcept { return use_count_before_; }

	inline const jive_resource_class_count &
	use_count_after() const noexcept { return use_count_after_; }

	/* Mutators - place node somewhere in region */

	/* remove from cut */
	void
	remove_from_cut();

	/* add to cut at specified location */
	void
	add_to_cut(jive_cut * cut, jive_shaped_node * before = nullptr);

private:
	/* private mutators to keep dependent data in sync */
	void
	add_ssavar_before(
		jive_shaped_ssavar * shaped_ssavar,
		jive_variable * variable,
		size_t count);
	void
	remove_ssavar_before(
		jive_shaped_ssavar * shaped_ssavar,
		jive_variable * variable,
		size_t count) noexcept;

	void
	add_ssavar_crossed(
		jive_shaped_ssavar * shaped_ssavar,
		jive_variable * variable,
		size_t count);
	void
	remove_ssavar_crossed(
		jive_shaped_ssavar * shaped_ssavar,
		jive_variable * variable,
		size_t count) noexcept;

	void
	add_ssavar_after(
		jive_shaped_ssavar * shaped_ssavar,
		jive_variable * variable,
		size_t count);
	void
	remove_ssavar_after(
		jive_shaped_ssavar * shaped_ssavar,
		jive_variable * variable,
		size_t count) noexcept;

	void
	inc_active_after(
		jive_nodevar_xpoint * xpoint,
		jive_shaped_ssavar * shaped_ssavar,
		jive_variable * variable,
		size_t count);

	void
	dec_active_after(
		jive_nodevar_xpoint * xpoint,
		jive_shaped_ssavar * shaped_ssavar,
		jive_variable * variable,
		size_t count) noexcept;

	void
	inc_active_before(
		jive_nodevar_xpoint * xpoint,
		jive_shaped_ssavar * shaped_ssavar,
		jive_variable * variable,
		size_t count);

	void
	dec_active_before(
		jive_nodevar_xpoint * xpoint,
		jive_shaped_ssavar * shaped_ssavar,
		jive_variable * variable,
		size_t count) noexcept;

	/* get (or create new) crossing point between this node and
	 * given shaped variable */
	inline jive_nodevar_xpoint *
	get_xpoint(jive_shaped_ssavar * shaped_ssavar)
	{
		auto i = ssavar_xpoints_.find(shaped_ssavar);
		return
			i != ssavar_xpoints_.end() ?
			i.ptr() :
			jive_nodevar_xpoint::create(this, shaped_ssavar);
	}

	void
	remove_all_crossed() noexcept;

	void
	add_crossings_from_lower_location(
		jive_shaped_graph * shaped_graph,
		jive_shaped_node * lower);

	jive_cut * cut_;
	jive_node * node_;
	jive_shaped_graph * shaped_graph_;
	
	jive_nodevar_xpoint_hash_byssavar ssavar_xpoints_;
	jive_resource_class_count use_count_before_;
	jive_resource_class_count use_count_after_;

	jive::detail::intrusive_hash_anchor<jive_shaped_node> hash_chain;
	jive::detail::intrusive_list_anchor<jive_shaped_node> cut_node_list_;

	typedef jive::detail::intrusive_hash_accessor<
		jive_node *,
		jive_shaped_node,
		&jive_shaped_node::node_,
		&jive_shaped_node::hash_chain
	> hash_chain_accessor;
	typedef jive::detail::intrusive_list_accessor<
		jive_shaped_node,
		&jive_shaped_node::cut_node_list_
	> list_accessor;

	friend class jive_cut;
	friend class jive_nodevar_xpoint;
	friend class jive_shaped_graph;
	friend class jive_shaped_region;
	friend class jive_shaped_ssavar;
	friend class jive_shaped_variable;
};

inline
jive_shaped_node_downward_iterator::jive_shaped_node_downward_iterator(
	jive_shaped_node * start)
	: visit_stack_({start}), shaped_graph_(&start->shaped_graph())
{
}

#endif
