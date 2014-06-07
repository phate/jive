/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_VARIABLE_H
#define JIVE_VSDG_VARIABLE_H

#include <stdbool.h>
#include <stddef.h>

#include <jive/common.h>
#include <jive/vsdg/region-ssavar-use.h>

namespace jive {
	class input;
}

/**
        \defgroup jive_variable Variables
        Variables
        @{
*/

typedef struct jive_ssavar jive_ssavar;
typedef struct jive_variable jive_variable;

struct jive_ssavar {
	size_t use_count;
	
	jive_variable * variable;
	struct {
		jive_ssavar * prev;
		jive_ssavar * next;
	} variable_ssavar_list;
	
	struct jive_output * origin;
	
	struct {
		jive::input * first;
		jive::input * last;
	} assigned_inputs;
	
	struct jive_output * assigned_output;
	
	struct {
		jive_ssavar * prev;
		jive_ssavar * next;
	} originating_ssavar_list;
	
	jive_ssavar_region_hash assigned_regions;
};

jive_ssavar *
jive_ssavar_create(struct jive_output * origin, jive_variable * variable);

void
jive_ssavar_assign_input(jive_ssavar * self, jive::input * input);

void
jive_ssavar_unassign_input(jive_ssavar * self, jive::input * input);

void
jive_ssavar_assign_output(jive_ssavar * self, struct jive_output * output);

void
jive_ssavar_unassign_output(jive_ssavar * self, struct jive_output * output);

void
jive_ssavar_merge(jive_ssavar * self, struct jive_ssavar * other);

void
jive_ssavar_divert_origin(jive_ssavar * self, struct jive_output * new_origin);

void
jive_ssavar_split(jive_ssavar * self);

void
jive_ssavar_destroy(jive_ssavar * self);

/**
	\brief Perform conistenty check on ssavar
	\param self ssavar to check
	
	Perform check whether the ssavar internal structure is
	consistent (and terminate process with an assertion failure
	otherwise).
*/
void
jive_ssavar_assert_consistent(const jive_ssavar * self);

struct jive_variable {
	struct jive_graph * graph;
	struct {
		jive_variable * prev;
		jive_variable * next;
	} graph_variable_list;
	
	size_t use_count;
	
	struct {
		jive_ssavar * first;
		jive_ssavar * last;
	} ssavars;
	
	struct {
		jive_ssavar * first;
		jive_ssavar * last;
	} unused_ssavars;
	
	struct {
		struct jive_gate * first;
		struct jive_gate * last;
	} gates;
	
	const struct jive_resource_class * rescls;
	const struct jive_resource_name * resname;
};

jive_variable *
jive_variable_create(struct jive_graph * graph);

void
jive_variable_assign_gate(jive_variable * self, struct jive_gate * gate);

void
jive_variable_unassign_gate(jive_variable * self, struct jive_gate * gate);

void
jive_variable_merge(jive_variable * self, jive_variable * other);

void
jive_variable_set_resource_class(jive_variable * self, const struct jive_resource_class * rescls);

JIVE_EXPORTED_INLINE const struct jive_resource_class *
jive_variable_get_resource_class(const jive_variable * self)
{
	return self->rescls;
}

void
jive_variable_recompute_rescls(jive_variable * self);

void
jive_variable_set_resource_name(jive_variable * self, const struct jive_resource_name * resname);

JIVE_EXPORTED_INLINE const struct jive_resource_name *
jive_variable_get_resource_name(const jive_variable * self)
{
	return self->resname;
}

bool
jive_variable_may_spill(const jive_variable * self);

JIVE_EXPORTED_INLINE size_t
jive_variable_used(const jive_variable * self)
{
	return self->use_count;
}

void
jive_variable_destroy(jive_variable * self);

/**	@}	*/

#endif
