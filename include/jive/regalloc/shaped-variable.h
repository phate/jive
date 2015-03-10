/*
 * Copyright 2010 2011 2012 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_SHAPED_VARIABLE_H
#define JIVE_REGALLOC_SHAPED_VARIABLE_H

#include <stdbool.h>

#include <unordered_set>

#include <jive/common.h>

#include <jive/regalloc/xpoint.h>
#include <jive/util/intrusive-hash.h>

class jive_nodevar_xpoint;
class jive_shaped_graph;
class jive_shaped_variable;
class jive_var_assignment_tracker;

struct jive_resource_name;
struct jive_ssavar;

class jive_shaped_variable {
public:
	struct interference_item {
		struct part {
			jive_shaped_variable * shaped_variable;
			interference_item * whole;
	
		private:
			jive::detail::intrusive_hash_anchor<part> hash_chain;
			typedef jive::detail::intrusive_hash_accessor<
				jive_shaped_variable *,
				part,
				&part::shaped_variable,
				&part::hash_chain
			> hash_chain_accessor;

			friend class jive_shaped_variable;
		};

		part first;
		part second;
		size_t count;
	};

	typedef jive::detail::intrusive_hash<
		const jive_shaped_variable *,
		interference_item::part,
		interference_item::part::hash_chain_accessor
	> interference_hash;

	~jive_shaped_variable();

	jive_shaped_variable(
		jive_shaped_graph * shaped_graph,
		jive_variable * variable);

	inline jive_variable *
	variable() const noexcept { return variable_; }

	inline jive_shaped_graph &
	shaped_graph() const noexcept { return *shaped_graph_; }

	const std::unordered_set<const jive_resource_name *> &
	allowed_names() const noexcept { return allowed_names_; }

	inline size_t
	squeeze() const noexcept { return squeeze_; }

	/* check interference of this variable with another; returns number
	 * of interference points (0 if none) */
	size_t
	interferes_with(const jive_shaped_variable * other) const noexcept;

	/* check whether the two variables can be merged */
	bool
	can_merge(const jive_variable * other) const noexcept;

	/* check whether resource class for this variable can be changed
	 * to the given class; returns the resource class that would be
	 * overflowing somewhere as a result of this change, or nullptr if
	 * the operation would not cause overflow */
	const jive_resource_class *
	check_change_resource_class(const jive_resource_class * new_rescls) const noexcept;

	/* test whether the given resource name is allowed for this variable */
	bool
	allowed_resource_name(const jive_resource_name * name) const noexcept;

	/* count number of allowed resource names */
	size_t
	allowed_resource_name_count() const noexcept;

	/* test whether variable is alive before point in shaped graph;
	 * returns "alive count" (0 if not alive) */
	size_t
	is_active_before(const jive_shaped_node * shaped_node) const noexcept;

	/* test whether variable crosses point in shaped graph;
	 * returns "alive count" (0 if not alive) */
	size_t
	is_crossing(const jive_shaped_node * shaped_node) const noexcept;

	/* test whether variable is alive after point in shaped graph;
	 * returns "alive count" (0 if not alive) */
	size_t
	is_active_after(const jive_shaped_node * shaped_node) const noexcept;

	static size_t
	interference_add(jive_shaped_variable * first, jive_shaped_variable * second);

	static size_t
	interference_remove(jive_shaped_variable * first, jive_shaped_variable * second);

	/**
		\brief Determine maximum cross count of this variable
		
		Visit all places at which this variable is alive, determine maximum
		of use counts per register class at these places.
	*/
	void
	get_cross_count(jive_resource_class_count * counts) const;

	interference_hash interference;

private:
	/* force recomputation of allowed names */
	void
	recompute_allowed_names();
	void
	internal_recompute_allowed_names();

	/* mark this name as forbidden for this variable */
	void
	deny_name(const jive_resource_name * resname);

	void
	resource_class_change(
		const jive_resource_class * old_rescls, const jive_resource_class * new_rescls);

	void
	resource_name_change(
		const jive_resource_name * old_resname, const jive_resource_name * new_resname);

	void
	initial_assign_gate(jive::gate * gate);

	void
	assign_gate(jive::gate * gate);

	void
	unassign_gate(jive::gate * gate);

	void
	add_squeeze(const jive_resource_class * rescls);

	void
	sub_squeeze(const jive_resource_class * rescls);

	jive_shaped_graph * shaped_graph_;
	jive_variable * variable_;

	struct {
		jive_shaped_variable * prev;
		jive_shaped_variable * next;
	} assignment_variable_list_;

	std::unordered_set<const jive_resource_name *> allowed_names_;
	size_t squeeze_;

	jive::detail::intrusive_hash_anchor<jive_shaped_variable> hash_chain;

	typedef jive::detail::intrusive_hash_accessor <
		jive_variable *,
		jive_shaped_variable,
		&jive_shaped_variable::variable_,
		&jive_shaped_variable::hash_chain
	> hash_chain_accessor;

	friend class jive_shaped_graph;
	friend class jive_var_assignment_tracker;
};

jive_shaped_variable *
jive_shaped_variable_create(
	jive_shaped_graph * shaped_graph,
	jive_variable * variable);


class jive_shaped_ssavar {
public:
	~jive_shaped_ssavar();

	inline
	jive_shaped_ssavar(jive_shaped_graph * shaped_graph, jive_ssavar * ssavar) noexcept
		: shaped_graph_(shaped_graph)
		, ssavar_(ssavar)
		, boundary_region_depth_(static_cast<size_t>(-1))
	{
	}

	inline jive_shaped_graph &
	shaped_graph() const noexcept { return *shaped_graph_; }

	inline jive_ssavar &
	ssavar() const noexcept { return *ssavar_; }

	inline size_t
	boundary_region_depth() const noexcept { return boundary_region_depth_; }

	void
	set_boundary_region_depth(size_t depth);

	inline void
	lower_boundary_region_depth(size_t depth)
	{
		if (depth < boundary_region_depth_) {
			set_boundary_region_depth(depth);
		}
	}

	size_t
	is_active_before(const jive_shaped_node * shaped_node) const noexcept;

	size_t
	is_crossing(const jive_shaped_node * shaped_node) const noexcept;

	size_t
	is_active_after(const jive_shaped_node * shaped_node) const noexcept;

private:
	typedef jive::detail::owner_intrusive_hash<
		const jive_shaped_node *,
		jive_nodevar_xpoint,
		jive_nodevar_xpoint::node_hash_accessor
	> jive_nodevar_xpoint_hash_bynode;

	/* arc (de)registration functions, with various special case
	 * treatments for end points */
	void
	xpoints_register_arc(
		jive_shaped_node * origin_shaped_node,
		jive_shaped_node * target_shaped_node);
	void
	xpoints_unregister_arc(
		jive_shaped_node * origin_shaped_node,
		jive_shaped_node * target_shaped_node) noexcept;

	void
	xpoints_register_region_arc(
		jive_shaped_node * origin_shaped_node,
		jive_shaped_node * target_shaped_node);
	void
	xpoints_unregister_region_arc(
		jive_shaped_node * origin_shaped_node,
		jive_shaped_node * target_shaped_node) noexcept;

	/* common arc (de)registration functions; the common part dealing with
	 * the "interior" */
	void
	common_xpoints_register_arc(
		jive_shaped_node * origin_shaped_node,
		jive_shaped_node * target_shaped_node,
		jive_variable * variable);
	void
	common_xpoints_unregister_arc(
		jive_shaped_node * origin_shaped_node,
		jive_shaped_node * target_shaped_node,
		jive_variable * variable) noexcept;

	/* (de)register all arcs belonging to this variable */
	void
	xpoints_register_arcs();
	void
	xpoints_unregister_arcs() noexcept;

	void
	xpoints_variable_change(
		jive_variable * old_variable,
		jive_variable * new_variable);

	void
	notify_divert_origin(
		jive::output * old_origin,
		jive::output * new_origin);

	void
	xpoints_change_resource_class(
		const jive_resource_class * old_rescls,
		const jive_resource_class * new_rescls);

	const jive_resource_class *
	check_change_resource_class(
		const jive_resource_class * old_rescls,
		const jive_resource_class * new_rescls) const noexcept;


	jive_shaped_graph * shaped_graph_;
	jive_ssavar * ssavar_;

	size_t boundary_region_depth_;

	jive_nodevar_xpoint_hash_bynode node_xpoints_;

	jive::detail::intrusive_hash_anchor<jive_shaped_ssavar> hash_chain_;

	typedef jive::detail::intrusive_hash_accessor <
		jive_ssavar *,
		jive_shaped_ssavar,
		&jive_shaped_ssavar::ssavar_,
		&jive_shaped_ssavar::hash_chain_
	> hash_chain_accessor;

	friend class jive_nodevar_xpoint;
	friend class jive_shaped_graph;
	friend class jive_shaped_node;
	friend class jive_shaped_variable;
};

#endif
