/*
 * Copyright 2010 2011 2012 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_XPOINT_H
#define JIVE_REGALLOC_XPOINT_H

#include <jive/util/intrusive-hash.h>
#include <jive/util/intrusive-list.h>
#include <jive/vsdg/resource.h>

namespace jive {
	class output;
}

struct jive_shaped_node;
struct jive_shaped_region;
struct jive_shaped_ssavar;

struct jive_variable;

struct jive_nodevar_xpoint {
	~jive_nodevar_xpoint() noexcept;

	jive_nodevar_xpoint(
		jive_shaped_node * shaped_node,
		jive_shaped_ssavar * shaped_ssavar) noexcept;

	jive_nodevar_xpoint(const jive_nodevar_xpoint &) = delete;
	void operator=(const jive_nodevar_xpoint &) = delete;

	inline jive_shaped_node & shaped_node() const noexcept { return *shaped_node_; }
	inline jive_shaped_ssavar & shaped_ssavar() const noexcept { return *shaped_ssavar_; }
	inline size_t before_count() const noexcept { return before_count_; }
	inline size_t cross_count() const noexcept { return cross_count_; }
	inline size_t after_count() const noexcept { return after_count_; }

private:
	static jive_nodevar_xpoint *
	create(
		jive_shaped_node * shaped_node,
		jive_shaped_ssavar * shaped_ssavar);

	inline void
	put() noexcept
	{
		if (before_count_ == 0 && after_count_ == 0) {
			destroy();
		}
	}

	void
	destroy() noexcept;

	jive_shaped_node * shaped_node_;
	jive_shaped_ssavar * shaped_ssavar_;
	
	size_t before_count_;
	size_t cross_count_;
	size_t after_count_;

	jive::detail::intrusive_hash_anchor<jive_nodevar_xpoint> node_hash_chain_;
	jive::detail::intrusive_hash_anchor<jive_nodevar_xpoint> ssavar_hash_chain_;

	typedef jive::detail::intrusive_hash_accessor<
		jive_shaped_ssavar *,
		jive_nodevar_xpoint,
		&jive_nodevar_xpoint::shaped_ssavar_,
		&jive_nodevar_xpoint::ssavar_hash_chain_
	> ssavar_hash_accessor;

	typedef jive::detail::intrusive_hash_accessor<
		jive_shaped_node *,
		jive_nodevar_xpoint,
		&jive_nodevar_xpoint::shaped_node_,
		&jive_nodevar_xpoint::node_hash_chain_
	> node_hash_accessor;

	friend class jive_shaped_node;
	friend class jive_shaped_ssavar;
};

class jive_cutvar_xpoint {
public:
	~jive_cutvar_xpoint() noexcept;

	jive_cutvar_xpoint(
		jive_shaped_ssavar * shaped_ssavar,
		jive::output * origin,
		jive_variable * variable,
		const jive_resource_class * rescls,
		size_t count) noexcept;

	inline jive_shaped_ssavar * shaped_ssavar() const noexcept { return shaped_ssavar_; }
	inline jive::output * origin() const noexcept { return origin_; }
	inline jive_variable * variable() const noexcept { return variable_; }
	inline const jive_resource_class * rescls() const noexcept { return rescls_; }
	inline size_t count() const noexcept { return count_; }

private:
	jive_shaped_ssavar * shaped_ssavar_;
	jive::output * origin_;
	jive_variable * variable_;
	const jive_resource_class * rescls_;
	size_t count_;

	jive::detail::intrusive_hash_anchor<jive_cutvar_xpoint> ssavar_hash_chain_;
	jive::detail::intrusive_hash_anchor<jive_cutvar_xpoint> origin_hash_chain_;
	jive::detail::intrusive_hash_anchor<jive_cutvar_xpoint> variable_hash_chain_;
	jive::detail::intrusive_list_anchor<jive_cutvar_xpoint> list_anchor_;

	typedef jive::detail::intrusive_hash_accessor<
		jive_shaped_ssavar *,
		jive_cutvar_xpoint,
		&jive_cutvar_xpoint::shaped_ssavar_,
		&jive_cutvar_xpoint::ssavar_hash_chain_
	> ssavar_hash_accessor;
	typedef jive::detail::intrusive_hash_accessor<
		jive::output *,
		jive_cutvar_xpoint,
		&jive_cutvar_xpoint::origin_,
		&jive_cutvar_xpoint::origin_hash_chain_
	> origin_hash_accessor;
	typedef jive::detail::intrusive_hash_accessor<
		jive_variable *,
		jive_cutvar_xpoint,
		&jive_cutvar_xpoint::variable_,
		&jive_cutvar_xpoint::variable_hash_chain_
	> variable_hash_accessor;
	typedef jive::detail::intrusive_list_accessor<
		jive_cutvar_xpoint,
		&jive_cutvar_xpoint::list_anchor_
	> list_accessor;

	friend class jive_varcut;
};

class jive_varcut {
public:
	typedef jive::detail::intrusive_hash<
		const jive_shaped_ssavar *,
		jive_cutvar_xpoint,
		jive_cutvar_xpoint::ssavar_hash_accessor
	> ssavar_map_type;
	typedef jive::detail::intrusive_hash<
		const jive::output *,
		jive_cutvar_xpoint,
		jive_cutvar_xpoint::origin_hash_accessor
	> origin_map_type;
	typedef jive::detail::intrusive_hash<
		const jive_variable *,
		jive_cutvar_xpoint,
		jive_cutvar_xpoint::variable_hash_accessor
	> variable_map_type;
	typedef jive::detail::owner_intrusive_list<
		jive_cutvar_xpoint,
		jive_cutvar_xpoint::list_accessor
	> xpoint_list;

	typedef jive_cutvar_xpoint xpoint_type;
	typedef jive_cutvar_xpoint value_type;
	typedef xpoint_list::const_iterator iterator;

	/* construction and assignment */
	inline jive_varcut() noexcept {}
	jive_varcut(const jive_varcut & other);
	inline jive_varcut(jive_varcut && other) noexcept { swap(other); }
	void swap(jive_varcut & other) noexcept;

	inline const jive_varcut &
	operator=(const jive_varcut & other) { jive_varcut(other).swap(*this); return *this; }
	inline const jive_varcut &
	operator=(jive_varcut && other) noexcept { swap(other); return *this; }

	/* iteration */
	inline iterator begin() const noexcept { return xpoints_.begin(); }
	inline iterator end() const noexcept { return xpoints_.end(); }

	/* querying */

	inline const ssavar_map_type &
	ssavar_map() const noexcept { return ssavar_map_; }
	inline const origin_map_type &
	origin_map() const noexcept { return origin_map_; }
	inline const variable_map_type &
	variable_map() const noexcept { return variable_map_; }
	inline const jive_resource_class_count &
	use_counts() const noexcept { return use_counts_; }

	inline size_t
	shaped_ssavar_is_active(jive_shaped_ssavar * shaped_ssavar) const noexcept
	{
		auto i = ssavar_map_.find(shaped_ssavar);
		return i != ssavar_map_.end() ? i->count() : 0;
	}

	inline size_t
	output_is_active(const jive::output * output) const noexcept
	{
		auto i = origin_map_.find(output);
		return i != origin_map_.end() ? i->count() : 0;
	}

	inline jive_shaped_ssavar *
	map_output(jive::output * output) const noexcept
	{
		auto i = origin_map_.find(output);
		return i != origin_map_.end() ? i->shaped_ssavar() : nullptr;
	}

	inline jive_shaped_ssavar *
	map_variable(jive_variable * variable) const noexcept
	{
		auto i = variable_map_.find(variable);
		return i != variable_map_.end() ? i->shaped_ssavar() : nullptr;
	}

	/* mutators */

	size_t
	ssavar_add(jive_shaped_ssavar * shaped_ssavar, size_t count);

	size_t
	ssavar_remove(jive_shaped_ssavar * shaped_ssavar, size_t count);

	void
	ssavar_remove_full(jive_shaped_ssavar * shaped_ssavar);

	void
	ssavar_divert_origin(jive_shaped_ssavar * shaped_ssavar, jive::output * origin);

	void
	ssavar_variable_change(jive_shaped_ssavar * shaped_ssavar, jive_variable * variable);

	void
	ssavar_rescls_change(jive_shaped_ssavar * shaped_ssavar, const jive_resource_class * rescls);

	void
	clear() noexcept;

	jive_cutvar_xpoint *
	create_xpoint(
		jive_shaped_ssavar * shaped_ssavar,
		jive::output * origin,
		jive_variable * variable,
		const jive_resource_class * rescls,
		size_t count);

	void
	remove_xpoint(jive_cutvar_xpoint * xpoint) noexcept;

private:
	ssavar_map_type ssavar_map_;
	origin_map_type origin_map_;
	variable_map_type variable_map_;
	jive_resource_class_count use_counts_;

	xpoint_list xpoints_;
};

#endif
