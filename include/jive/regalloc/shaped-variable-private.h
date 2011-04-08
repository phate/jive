#ifndef JIVE_REGALLOC_SHAPED_VARIABLE_PRIVATE_H
#define JIVE_REGALLOC_SHAPED_VARIABLE_PRIVATE_H

#include <stdlib.h>

#include <jive/common.h>
#include <jive/context.h>
#include <jive/regalloc/assignment-tracker-private.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-variable.h>
#include <jive/vsdg/resource-private.h>
#include <jive/vsdg/variable.h>

struct jive_gate;
struct jive_input;
struct jive_output;
struct jive_resource_class;

struct jive_variable_interference_part {
	jive_shaped_variable * shaped_variable;
	struct {
		jive_variable_interference_part * prev;
		jive_variable_interference_part * next;
	} chain;
	jive_variable_interference * whole;
};

struct jive_variable_interference {
	jive_variable_interference_part first;
	jive_variable_interference_part second;
	size_t count;
};

JIVE_DEFINE_HASH_TYPE(jive_variable_interference_hash, struct jive_variable_interference_part, struct jive_shaped_variable *, shaped_variable, chain);

jive_variable_interference *
jive_variable_interference_create(jive_shaped_variable * first, jive_shaped_variable * second);

void
jive_variable_interference_destroy(jive_variable_interference * self);

typedef struct jive_allowed_resource_name jive_allowed_resource_name;
struct jive_allowed_resource_name {
	const struct jive_resource_name * name;
	struct {
		jive_allowed_resource_name* prev;
		jive_allowed_resource_name* next;
	} chain;
};
JIVE_DEFINE_HASH_TYPE(jive_allowed_resource_names_hash, jive_allowed_resource_name, const struct jive_resource_name *, name, chain);

static void
jive_allowed_resource_names_add(jive_allowed_resource_names_hash * hash, const jive_resource_name * resource_name)
{
	jive_allowed_resource_name * allowed = jive_context_malloc(hash->context, sizeof(*allowed));
	allowed->name = resource_name;
	jive_allowed_resource_names_hash_insert(hash, allowed);
}

static void
jive_allowed_resource_name_destroy(jive_allowed_resource_names_hash * hash, jive_allowed_resource_name * allowed)
{
	jive_allowed_resource_names_hash_remove(hash, allowed);
	jive_context_free(hash->context, allowed);
}

static void
jive_allowed_resource_names_remove(jive_allowed_resource_names_hash * hash, const struct jive_resource_name * resource_name)
{
	jive_allowed_resource_name * allowed = jive_allowed_resource_names_hash_lookup(hash, resource_name);
	if (!allowed) return;
	jive_allowed_resource_name_destroy(hash, allowed);
}

static void
jive_allowed_resource_names_clear(jive_allowed_resource_names_hash * hash)
{
	struct jive_allowed_resource_names_hash_iterator i;
	i = jive_allowed_resource_names_hash_begin(hash);
	while(i.entry) {
		jive_allowed_resource_name * allowed = i.entry;
		jive_allowed_resource_names_hash_iterator_next(&i);
		jive_allowed_resource_name_destroy(hash, allowed);
	}
}

static inline void
jive_shaped_variable_internal_recompute_allowed_names(jive_shaped_variable * self)
{
	self->squeeze = 0;
	jive_allowed_resource_names_clear(&self->allowed_names);
	
	if (self->variable->resname) {
		jive_allowed_resource_names_add(&self->allowed_names, self->variable->resname);
	} else if (self->variable->rescls->limit) {
		size_t nnames;
		const jive_resource_name * const * names;
		jive_resource_class_get_resource_names(self->variable->rescls, &nnames, &names);
		size_t n;
		for(n = 0; n < nnames; n++)
			jive_allowed_resource_names_add(&self->allowed_names, names[n]);
		
		struct jive_variable_interference_hash_iterator i;
		JIVE_HASH_ITERATE(jive_variable_interference_hash, self->interference, i) {
			jive_shaped_variable * other = i.entry->shaped_variable;
			if (other->variable->resname) {
				jive_allowed_resource_names_remove(&self->allowed_names, other->variable->resname);
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

static inline size_t
jive_variable_interference_add(jive_shaped_variable * first, jive_shaped_variable * second)
{
	jive_variable_interference * i;
	jive_variable_interference_part * part = jive_variable_interference_hash_lookup(&first->interference, second);
	if (part) i = part->whole;
	else {
		const jive_resource_name * first_name = first->variable->resname;
		const jive_resource_name * second_name = second->variable->resname;
		
		if (second_name)
			jive_allowed_resource_names_remove(&first->allowed_names, second_name);
		if (first_name)
			jive_allowed_resource_names_remove(&second->allowed_names, first_name);
		
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
	jive_variable_interference * i;
	jive_variable_interference_part * part = jive_variable_interference_hash_lookup(&first->interference, second);
	i = part->whole;
	size_t count = -- (i->count);
	if (!i->count) {
		jive_variable_interference_destroy(i);
		const jive_resource_name * first_name = first->variable->resname;
		const jive_resource_name * second_name = second->variable->resname;
		
		if (first_name || second_name) {
			jive_shaped_variable_internal_recompute_allowed_names(first);
			jive_shaped_variable_internal_recompute_allowed_names(second);
		} else {
			jive_shaped_variable_sub_squeeze(first, second->variable->rescls);
			jive_shaped_variable_sub_squeeze(second, first->variable->rescls);
		}
	}
	return count;
}

void
jive_shaped_variable_initial_assign_gate(jive_shaped_variable * self, struct jive_gate * gate);

void
jive_shaped_variable_assign_gate(jive_shaped_variable * self, struct jive_gate * gate);

void
jive_shaped_variable_unassign_gate(jive_shaped_variable * self, struct jive_gate * gate);

void
jive_shaped_variable_resource_class_change(jive_shaped_variable * self, const struct jive_resource_class * old_rescls, const struct jive_resource_class * new_rescls);

void
jive_shaped_variable_resource_name_change(jive_shaped_variable * self, const struct jive_resource_name * old_resname, const struct jive_resource_name * new_resname);

void
jive_shaped_ssavar_xpoints_register_arc(jive_shaped_ssavar * self, struct jive_input * input, struct jive_output * output);

void
jive_shaped_ssavar_xpoints_unregister_arc(jive_shaped_ssavar * self, struct jive_input * input, struct jive_output * output);

void
jive_shaped_ssavar_xpoints_register_region_arc(jive_shaped_ssavar * self, struct jive_output * output, struct jive_region * region);

void
jive_shaped_ssavar_xpoints_unregister_region_arc(jive_shaped_ssavar * self, struct jive_output * output, struct jive_region * region);

void
jive_shaped_ssavar_xpoints_register_arcs(jive_shaped_ssavar * self);

void
jive_shaped_ssavar_xpoints_unregister_arcs(jive_shaped_ssavar * self);

void
jive_shaped_ssavar_xpoints_variable_change(jive_shaped_ssavar * self, jive_variable * old_variable, jive_variable * new_variable);

void
jive_shaped_ssavar_xpoints_change_resource_class(jive_shaped_ssavar * self, const struct jive_resource_class * old_rescls, const struct jive_resource_class * new_rescls);

const struct jive_resource_class *
jive_shaped_ssavar_check_change_resource_class(const jive_shaped_ssavar * self, const struct jive_resource_class * old_rescls, const struct jive_resource_class * new_rescls);

#endif
