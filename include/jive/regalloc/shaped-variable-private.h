/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_SHAPED_VARIABLE_PRIVATE_H
#define JIVE_REGALLOC_SHAPED_VARIABLE_PRIVATE_H

#include <stdlib.h>

#include <jive/common.h>
#include <jive/regalloc/assignment-tracker-private.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-variable.h>
#include <jive/vsdg/resource-private.h>
#include <jive/vsdg/variable.h>

namespace jive {
	class gate;
	class output;
}

struct jive_resource_class;

jive_variable_interference *
jive_variable_interference_create(jive_shaped_variable * first, jive_shaped_variable * second);

void
jive_variable_interference_destroy(jive_variable_interference * self);

static inline void
jive_shaped_variable_internal_recompute_allowed_names(jive_shaped_variable * self)
{
	self->squeeze = 0;
	self->allowed_names.clear();
	
	if (self->variable->resname) {
		self->allowed_names.insert(self->variable->resname);
	} else if (self->variable->rescls->limit) {
		size_t nnames;
		const jive_resource_name * const * names;
		jive_resource_class_get_resource_names(self->variable->rescls, &nnames, &names);
		size_t n;
		for(n = 0; n < nnames; n++)
			self->allowed_names.insert(names[n]);
		
		for (const jive_variable_interference_part & part : self->interference) {
			jive_shaped_variable * other = part.shaped_variable;
			if (other->variable->resname) {
				self->allowed_names.erase(other->variable->resname);
			} else if (other->variable->rescls->limit) {
				const jive_resource_class * rescls;
				rescls = jive_resource_class_intersection(self->variable->rescls, other->variable->rescls);
				if (rescls)
					self->squeeze ++;
			}
		}
	}
}

static inline void
jive_shaped_variable_recompute_allowed_names(jive_shaped_variable * self)
{
	jive_var_assignment_tracker_remove_tracked(&self->shaped_graph->var_assignment_tracker,
		self, self->variable->rescls, self->variable->resname);
	jive_shaped_variable_internal_recompute_allowed_names(self);
	jive_var_assignment_tracker_add_tracked(&self->shaped_graph->var_assignment_tracker,
		self, self->variable->rescls, self->variable->resname);
}

static inline void
jive_shaped_variable_add_squeeze(jive_shaped_variable * self, const jive_resource_class * rescls)
{
	if (self->variable->resname || !self->variable->rescls->limit || !rescls->limit)
		return;
	jive_var_assignment_tracker_remove_tracked(&self->shaped_graph->var_assignment_tracker,
		self, self->variable->rescls, self->variable->resname);
	if (jive_resource_class_intersection(self->variable->rescls, rescls))
		self->squeeze ++;
	jive_var_assignment_tracker_add_tracked(&self->shaped_graph->var_assignment_tracker,
		self, self->variable->rescls, self->variable->resname);
}

static inline void
jive_shaped_variable_sub_squeeze(jive_shaped_variable * self, const jive_resource_class * rescls)
{
	if (self->variable->resname || !self->variable->rescls->limit || !rescls->limit)
		return;
	jive_var_assignment_tracker_remove_tracked(&self->shaped_graph->var_assignment_tracker,
		self, self->variable->rescls, self->variable->resname);
	if (jive_resource_class_intersection(self->variable->rescls, rescls)) {
		JIVE_DEBUG_ASSERT(self->squeeze > 0);
		self->squeeze --;
	}
	jive_var_assignment_tracker_add_tracked(&self->shaped_graph->var_assignment_tracker,
		self, self->variable->rescls, self->variable->resname);
}

void
jive_shaped_variable_deny_name(jive_shaped_variable * self,
	const struct jive_resource_name * resname);

static inline size_t
jive_variable_interference_add(jive_shaped_variable * first, jive_shaped_variable * second)
{
	jive_variable_interference * i;
	auto iter = first->interference.find(second);
	if (iter != first->interference.end()) {
		i = iter->whole;
	} else {
		const jive_resource_name * first_name = first->variable->resname;
		const jive_resource_name * second_name = second->variable->resname;
		
		if (second_name)
			jive_shaped_variable_deny_name(first, second_name);
		if (first_name)
			jive_shaped_variable_deny_name(second, first_name);
		
		if (!second_name)
			jive_shaped_variable_add_squeeze(first, second->variable->rescls);
		if (!first_name)
			jive_shaped_variable_add_squeeze(second, first->variable->rescls);
		
		i = jive_variable_interference_create(first, second);
	}
	return i->count ++;
}

static inline size_t
jive_variable_interference_remove(jive_shaped_variable * first, jive_shaped_variable * second)
{
	auto iter = first->interference.find(second);
	jive_variable_interference_part * part = iter.ptr();
	jive_variable_interference * i = part->whole;
	size_t count = -- (i->count);
	if (!i->count) {
		jive_variable_interference_destroy(i);
		const jive_resource_name * first_name = first->variable->resname;
		const jive_resource_name * second_name = second->variable->resname;
		
		if (first_name || second_name) {
			jive_shaped_variable_recompute_allowed_names(first);
			jive_shaped_variable_recompute_allowed_names(second);
		} else {
			jive_shaped_variable_sub_squeeze(first, second->variable->rescls);
			jive_shaped_variable_sub_squeeze(second, first->variable->rescls);
		}
	}
	return count;
}

void
jive_shaped_variable_initial_assign_gate(jive_shaped_variable * self, jive::gate * gate);

void
jive_shaped_variable_assign_gate(jive_shaped_variable * self, jive::gate * gate);

void
jive_shaped_variable_unassign_gate(jive_shaped_variable * self, jive::gate * gate);

void
jive_shaped_variable_resource_class_change(jive_shaped_variable * self,
	const struct jive_resource_class * old_rescls, const struct jive_resource_class * new_rescls);

void
jive_shaped_variable_resource_name_change(jive_shaped_variable * self,
	const struct jive_resource_name * old_resname, const struct jive_resource_name * new_resname);

void
jive_shaped_ssavar_xpoints_register_arc(jive_shaped_ssavar * self, jive::input * input,
	jive::output * output);

void
jive_shaped_ssavar_xpoints_unregister_arc(jive_shaped_ssavar * self, jive::input * input,
	jive::output * output);

void
jive_shaped_ssavar_xpoints_register_region_arc(jive_shaped_ssavar * self, jive::output * output,
	struct jive_region * region);

void
jive_shaped_ssavar_xpoints_unregister_region_arc(jive_shaped_ssavar * self, jive::output * output,
	struct jive_region * region);

void
jive_shaped_ssavar_xpoints_register_arcs(jive_shaped_ssavar * self);

void
jive_shaped_ssavar_xpoints_unregister_arcs(jive_shaped_ssavar * self);

void
jive_shaped_ssavar_xpoints_variable_change(jive_shaped_ssavar * self, jive_variable * old_variable,
	jive_variable * new_variable);

void
jive_shaped_ssavar_notify_divert_origin(jive_shaped_ssavar * self, jive::output * old_origin,
	jive::output * new_origin);

void
jive_shaped_ssavar_xpoints_change_resource_class(jive_shaped_ssavar * self,
	const struct jive_resource_class * old_rescls, const struct jive_resource_class * new_rescls);

const struct jive_resource_class *
jive_shaped_ssavar_check_change_resource_class(const jive_shaped_ssavar * self,
	const struct jive_resource_class * old_rescls, const struct jive_resource_class * new_rescls);

#endif
