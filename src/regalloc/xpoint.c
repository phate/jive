/*
 * Copyright 2010 2011 2012 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/regalloc/xpoint.h>

#include <sstream>

#include <jive/common.h>

#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-node.h>
#include <jive/regalloc/shaped-variable.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/variable.h>

jive_nodevar_xpoint::~jive_nodevar_xpoint() noexcept
{
	shaped_node_->ssavar_xpoints_.erase(this);
}

jive_nodevar_xpoint::jive_nodevar_xpoint(
	jive_shaped_node * shaped_node,
	jive_shaped_ssavar * shaped_ssavar) noexcept
	: shaped_node_(shaped_node)
	, shaped_ssavar_(shaped_ssavar)
	, before_count_(0)
	, cross_count_(0)
	, after_count_(0)
{
	shaped_node_->ssavar_xpoints_.insert(this);
}

void
jive_nodevar_xpoint::destroy() noexcept
{
	shaped_ssavar_->node_xpoints_.erase(this);
}


jive_nodevar_xpoint *
jive_nodevar_xpoint::create(
	jive_shaped_node * shaped_node,
	jive_shaped_ssavar * shaped_ssavar)
{
	return shaped_ssavar->node_xpoints_.insert(std::unique_ptr<jive_nodevar_xpoint>(
		new jive_nodevar_xpoint(shaped_node, shaped_ssavar))).ptr();
}


jive_cutvar_xpoint::jive_cutvar_xpoint(
	jive_shaped_ssavar * shaped_ssavar,
	jive::output * origin,
	jive_variable * variable,
	const jive_resource_class * rescls,
	size_t count) noexcept
	: shaped_ssavar_(shaped_ssavar)
	, origin_(origin)
	, variable_(variable)
	, rescls_(rescls)
	, count_(count)
{
}

jive_cutvar_xpoint::~jive_cutvar_xpoint() noexcept
{
}

jive_varcut::jive_varcut(const jive_varcut & other)
{
	for (const jive_cutvar_xpoint & src_xpoint : other.xpoints_) {
		create_xpoint(
			src_xpoint.shaped_ssavar(),
			src_xpoint.origin(),
			src_xpoint.variable(),
			src_xpoint.rescls(),
			src_xpoint.count());
	}
}

void jive_varcut::swap(jive_varcut & other) noexcept
{
	ssavar_map_.swap(other.ssavar_map_);
	origin_map_.swap(other.origin_map_);
	variable_map_.swap(other.variable_map_);
	use_counts_.swap(other.use_counts_);
	xpoints_.swap(other.xpoints_);
}

jive_cutvar_xpoint *
jive_varcut::create_xpoint(
	jive_shaped_ssavar * shaped_ssavar,
	jive::output * origin,
	jive_variable * variable,
	const jive_resource_class * rescls,
	size_t count)
{
	std::unique_ptr<jive_cutvar_xpoint> xpoint(new jive_cutvar_xpoint(
		shaped_ssavar, origin, variable, rescls, count));

	/* note: if bad_alloc is thrown during any of the insertions,
	 * then this object will not be consistent anymore; it can only
	 * be safely destroyed, but not used otherwise */
	ssavar_map_.insert(xpoint.get());
	origin_map_.insert(xpoint.get());
	variable_map_.insert(xpoint.get());
	use_counts_.add(rescls);

	return xpoints_.insert(xpoints_.end(), std::move(xpoint)).ptr();
}

void
jive_varcut::remove_xpoint(jive_cutvar_xpoint * xpoint) noexcept
{
	ssavar_map_.erase(xpoint);
	origin_map_.erase(xpoint);
	variable_map_.erase(xpoint);
	use_counts_.sub(xpoint->rescls_);
	xpoints_.erase(xpoint);
}

void
jive_varcut::clear() noexcept
{
	use_counts_.clear();
	ssavar_map_.clear();
	origin_map_.clear();
	variable_map_.clear();
	xpoints_.clear();
}

size_t
jive_varcut::ssavar_add(
	jive_shaped_ssavar * shaped_ssavar,
	size_t count)
{
	auto i = ssavar_map_.find(shaped_ssavar);

	if (!count) {
		return i != ssavar_map_.end() ? i->count() : 0;
	}

	jive_cutvar_xpoint * xpoint;
	if (i == ssavar_map_.end()) {
		xpoint = create_xpoint(
			shaped_ssavar,
			shaped_ssavar->ssavar().origin,
			shaped_ssavar->ssavar().variable,
			jive_variable_get_resource_class(shaped_ssavar->ssavar().variable),
			0);
	} else {
		xpoint = i.ptr();
	}
	
	size_t old_count = xpoint->count_;
	xpoint->count_ += count;
	return old_count;
}

size_t
jive_varcut::ssavar_remove(
	jive_shaped_ssavar * shaped_ssavar,
	size_t count)
{
	auto i = ssavar_map_.find(shaped_ssavar);
	if (i == ssavar_map_.end()) {
		return 0;
	}

	size_t old_count = i->count_;
	if (count >= i->count_) {
		remove_xpoint(i.ptr());
	} else {
		i->count_ -= count;
	}
	
	return old_count;
}

void
jive_varcut::ssavar_remove_full(
	jive_shaped_ssavar * shaped_ssavar)
{
	auto i = ssavar_map_.find(shaped_ssavar);
	if (i != ssavar_map_.end()) {
		remove_xpoint(i.ptr());
	}
}

void
jive_varcut::ssavar_divert_origin(
	jive_shaped_ssavar * shaped_ssavar, jive::output * origin)
{
	auto i = ssavar_map_.find(shaped_ssavar);
	if (i != ssavar_map_.end()) {
		jive_cutvar_xpoint * xpoint = i.ptr();
		origin_map_.erase(xpoint);
		xpoint->origin_ = origin;
		origin_map_.insert(xpoint);
	}
}

void
jive_varcut::ssavar_variable_change(
	jive_shaped_ssavar * shaped_ssavar,
	jive_variable * variable)
{
	auto i = ssavar_map_.find(shaped_ssavar);
	if (i != ssavar_map_.end()) {
		jive_cutvar_xpoint * xpoint = i.ptr();
		variable_map_.erase(xpoint);
		xpoint->variable_ = variable;
		variable_map_.insert(xpoint);
	}
}

void
jive_varcut::ssavar_rescls_change(
	jive_shaped_ssavar * shaped_ssavar,
	const jive_resource_class * rescls)
{
	auto i = ssavar_map_.find(shaped_ssavar);
	if (i != ssavar_map_.end()) {
		use_counts_.change(i->rescls_, rescls);
		i->rescls_ = rescls;
	}
}

std::string
jive_varcut::debug_string() const
{
	std::ostringstream os;
	bool first = true;
	for (const jive_cutvar_xpoint & xpoint : xpoints_) {
		if (first) {
			first = false;
		} else {
			os << ", ";
		}
		os << "<" << xpoint.origin() << "," << xpoint.rescls()->name << "," << xpoint.count() << ">";
	}
	return os.str();
}
