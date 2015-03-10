/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_ASSIGNMENT_TRACKER_H
#define JIVE_REGALLOC_ASSIGNMENT_TRACKER_H

#include <stddef.h>

#include <vector>

#include <jive/common.h>

class jive_resource_class;
class jive_resource_name;
class jive_shaped_variable;

struct jive_pressured_var_list {
	jive_shaped_variable * first;
	jive_shaped_variable * last;
};

class jive_var_assignment_tracker {
public:
	inline
	~jive_var_assignment_tracker() {
		JIVE_DEBUG_ASSERT(pressured.empty());
		JIVE_DEBUG_ASSERT(assigned.first == 0);
		JIVE_DEBUG_ASSERT(trivial.first == 0);
	}

	inline
	jive_var_assignment_tracker() noexcept
		: assigned({0, 0})
		, trivial({0, 0})
	{
	}

	void
	add_tracked(
		jive_shaped_variable * shaped_variable,
		const jive_resource_class * rescls,
		const jive_resource_name * resname);

	void
	remove_tracked(
		jive_shaped_variable * shaped_variable,
		const jive_resource_class * rescls,
		const jive_resource_name * resname);

	struct {
		jive_shaped_variable * first;
		jive_shaped_variable * last;
	} assigned;
	struct {
		jive_shaped_variable * first;
		jive_shaped_variable * last;
	} trivial;
	std::vector<jive_pressured_var_list> pressured;
};

#endif
