#ifndef JIVE_VSDG_VARIABLE_H
#define JIVE_VSDG_VARIABLE_H

#include <stdbool.h>
#include <stddef.h>

#include <jive/vsdg/region-ssavar-use.h>

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
		struct jive_input * first;
		struct jive_input * last;
	} assigned_inputs;
	
	struct jive_output * assigned_output;
	
	jive_ssavar_region_hash assigned_regions;
};

jive_ssavar *
jive_ssavar_create(struct jive_output * origin, jive_variable * variable);

void
jive_ssavar_assign_input(jive_ssavar * self, struct jive_input * input);

void
jive_ssavar_unassign_input(jive_ssavar * self, struct jive_input * input);

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

inline const struct jive_resource_class *
jive_variable_get_resource_class(const jive_variable * self)
{
	return self->rescls;
}

void
jive_variable_recompute_rescls(jive_variable * self);

void
jive_variable_set_resource_name(jive_variable * self, const struct jive_resource_name * resname);

inline const struct jive_resource_name *
jive_variable_get_resource_name(const jive_variable * self)
{
	return self->resname;
}

bool
jive_variable_conflicts_with(const jive_variable * self, const jive_variable * other);

bool
jive_variable_may_spill(const jive_variable * self);

static inline size_t
jive_variable_used(const jive_variable * self)
{
	return self->use_count;
}

void
jive_variable_destroy(jive_variable * self);

#if 0
struct jive_resource_class {
	const struct jive_resource_class * parent;
	
	void (*fini)(jive_resource * self);
	
	char * (*get_label)(const jive_resource * self);
	
	const jive_type * (*get_type)(const jive_resource * self);
	
	bool (*can_merge)(const jive_resource * self, const jive_resource * other);
		
	void (*merge)(jive_resource * self, jive_resource * other);
	
	const struct jive_cpureg * (*get_cpureg)(const jive_resource * self);
	
	const struct jive_regcls * (*get_regcls)(const jive_resource * self);
	
	const struct jive_regcls * (*get_real_regcls)(const jive_resource * self);
	
	void (*add_squeeze)(jive_resource * self, const struct jive_regcls * regcls);
	
	void (*sub_squeeze)(jive_resource * self, const struct jive_regcls * regcls);
	
	void (*deny_register)(jive_resource * self, const struct jive_cpureg * reg);
	
	void (*recompute_allowed_registers)(jive_resource * self);
};

extern const struct jive_resource_class JIVE_RESOURCE;

/* FIXME: the following names could be regularized */

size_t
jive_resource_is_active_before(const jive_resource * self, const struct jive_node * node);

size_t
jive_resource_crosses(const jive_resource * self, const struct jive_node * node);

size_t
jive_resource_is_active_after(const jive_resource * self, const struct jive_node * node);

size_t
jive_resource_originates_in(const jive_resource * self, const struct jive_node * node);

size_t
jive_resource_is_used_by(const jive_resource * self, const struct jive_node * node);

size_t
jive_resource_interferes_with(const jive_resource * self, const jive_resource * other);

void
jive_resource_assign_input(jive_resource * self, jive_input * input);

void
jive_resource_unassign_input(jive_resource * self, jive_input * input);

void
jive_resource_assign_output(jive_resource * self, jive_output * output);

void
jive_resource_unassign_output(jive_resource * self, jive_output * output);

void
jive_resource_assign_gate(jive_resource * self, jive_gate * gate);

void
jive_resource_unassign_gate(jive_resource * self, jive_gate * gate);

#endif

/**	@}	*/

#endif
