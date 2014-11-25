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

struct jive_shaped_variable;

struct jive_pressured_var_list {
	struct jive_shaped_variable * first;
	struct jive_shaped_variable * last;
};

struct jive_var_assignment_tracker {
	inline
	~jive_var_assignment_tracker() {
		JIVE_DEBUG_ASSERT(pressured.empty());
		JIVE_DEBUG_ASSERT(assigned.first == 0);
		JIVE_DEBUG_ASSERT(trivial.first == 0);
	}

	struct {
		struct jive_shaped_variable * first;
		struct jive_shaped_variable * last;
	} assigned;
	struct {
		struct jive_shaped_variable * first;
		struct jive_shaped_variable * last;
	} trivial;
	std::vector<jive_pressured_var_list> pressured;
};

#endif
