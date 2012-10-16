/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_ASSIGNMENT_TRACKER_H
#define JIVE_REGALLOC_ASSIGNMENT_TRACKER_H

#include <stddef.h>

typedef struct jive_var_assignment_tracker jive_var_assignment_tracker;
typedef struct jive_pressured_var_list jive_pressured_var_list;

struct jive_context;
struct jive_shaped_variable;

struct jive_pressured_var_list {
	struct jive_shaped_variable * first;
	struct jive_shaped_variable * last;
};

struct jive_var_assignment_tracker {
	struct jive_context * context;
	
	struct {
		struct jive_shaped_variable * first;
		struct jive_shaped_variable * last;
	} assigned;
	struct {
		struct jive_shaped_variable * first;
		struct jive_shaped_variable * last;
	} trivial;
	jive_pressured_var_list * pressured;
	
	size_t pressured_max, pressured_space;
};

#endif
