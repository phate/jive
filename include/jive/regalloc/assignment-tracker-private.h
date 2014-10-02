/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_ASSIGNMENT_TRACKER_PRIVATE_H
#define JIVE_REGALLOC_ASSIGNMENT_TRACKER_PRIVATE_H

#include <jive/context.h>
#include <jive/regalloc/assignment-tracker.h>
#include <jive/regalloc/shaped-variable.h>
#include <jive/vsdg/resource.h>

#include <jive/common.h>

static inline void
jive_var_assignment_tracker_init(jive_var_assignment_tracker * self, jive_context * context)
{
	self->context = context;
	self->assigned.first = self->assigned.last = 0;
	self->trivial.first = self->trivial.last = 0;
}

static inline void
jive_var_assignment_tracker_add_tracked(jive_var_assignment_tracker * self, jive_shaped_variable * shaped_variable,
	const jive_resource_class * rescls, const jive_resource_name * resname)
{
	if (resname || rescls->limit == 0) {
		JIVE_LIST_PUSH_BACK(self->assigned, shaped_variable, assignment_variable_list);
	} else if (shaped_variable->allowed_names.size() > shaped_variable->squeeze) {
		JIVE_LIST_PUSH_BACK(self->trivial, shaped_variable, assignment_variable_list);
	} else {
		size_t index = shaped_variable->squeeze - shaped_variable->allowed_names.size();
		if (index >= self->pressured.size()) {
			self->pressured.resize(index + 1, {nullptr, nullptr});
		}
		JIVE_LIST_PUSH_BACK(self->pressured[index], shaped_variable, assignment_variable_list);
	}
}

static inline void
jive_var_assignment_tracker_remove_tracked(jive_var_assignment_tracker * self, jive_shaped_variable * shaped_variable,
	const jive_resource_class * rescls, const jive_resource_name * resname)
{
	if (resname || rescls->limit == 0) {
		JIVE_LIST_REMOVE(self->assigned, shaped_variable, assignment_variable_list);
	} else if (shaped_variable->allowed_names.size() > shaped_variable->squeeze) {
		JIVE_LIST_REMOVE(self->trivial, shaped_variable, assignment_variable_list);
	} else {
		size_t index = shaped_variable->squeeze - shaped_variable->allowed_names.size();
		JIVE_LIST_REMOVE(self->pressured[index], shaped_variable, assignment_variable_list);
		while (!self->pressured.empty()) {
			if (self->pressured.rbegin()->first) {
				break;
			}
			self->pressured.resize(self->pressured.size() -1);
		}
	}
}

#endif
