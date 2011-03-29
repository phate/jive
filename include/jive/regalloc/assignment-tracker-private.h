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
	self->pressured = 0;
	self->pressured_max = self->pressured_space = 0;
}

static inline void
jive_var_assignment_tracker_fini(jive_var_assignment_tracker * self)
{
	JIVE_DEBUG_ASSERT(self->pressured_max == 0);
	JIVE_DEBUG_ASSERT(self->assigned.first == 0);
	JIVE_DEBUG_ASSERT(self->trivial.first == 0);
	jive_context_free(self->context, self->pressured);
}

static inline void
jive_var_assignment_tracker_add_tracked(jive_var_assignment_tracker * self, jive_shaped_variable * shaped_variable,
	const jive_resource_class * rescls, const jive_resource_name * resname)
{
	if (resname || rescls->limit == 0) {
		JIVE_LIST_PUSH_BACK(self->assigned, shaped_variable, assignment_variable_list);
	} else if (shaped_variable->allowed_names.nitems > shaped_variable->squeeze) {
		JIVE_LIST_PUSH_BACK(self->trivial, shaped_variable, assignment_variable_list);
	} else {
		size_t index = shaped_variable->squeeze - shaped_variable->allowed_names.nitems;
		if (index >= self->pressured_space) {
			self->pressured = jive_context_realloc(self->context, self->pressured, sizeof(self->pressured[0]) * (index + 1));
			size_t n;
			for(n = self->pressured_space; n < index + 1; n++) {
				self->pressured[n].first = self->pressured[n].last = 0;
			}
			self->pressured_space = index + 1;
		}
		JIVE_LIST_PUSH_BACK(self->pressured[index], shaped_variable, assignment_variable_list);
		if (index + 1 > self->pressured_max)
			self->pressured_max = index + 1;
	}
}

static inline void
jive_var_assignment_tracker_remove_tracked(jive_var_assignment_tracker * self, jive_shaped_variable * shaped_variable,
	const jive_resource_class * rescls, const jive_resource_name * resname)
{
	if (resname || rescls->limit == 0) {
		JIVE_LIST_REMOVE(self->assigned, shaped_variable, assignment_variable_list);
	} else if (shaped_variable->allowed_names.nitems > shaped_variable->squeeze) {
		JIVE_LIST_REMOVE(self->trivial, shaped_variable, assignment_variable_list);
	} else {
		size_t index = shaped_variable->squeeze - shaped_variable->allowed_names.nitems;
		JIVE_LIST_REMOVE(self->pressured[index], shaped_variable, assignment_variable_list);
		while(self->pressured_max) {
			if (self->pressured[self->pressured_max - 1].first) break;
			self->pressured_max --;
		}
	}
}

#endif
