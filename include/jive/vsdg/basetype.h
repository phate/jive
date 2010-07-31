#ifndef JIVE_VSDG_BASETYPE_H
#define JIVE_VSDG_BASETYPE_H

#include <stdlib.h>
#include <stdbool.h>

#include <jive/vsdg/crossings.h>
#include <jive/vsdg/resource-interference.h>

typedef struct jive_type_class jive_type_class;
typedef struct jive_type jive_type;

typedef struct jive_input_class jive_input_class;
typedef struct jive_input jive_input;

typedef struct jive_output_class jive_output_class;
typedef struct jive_output jive_output;

typedef struct jive_gate_class jive_gate_class;
typedef struct jive_gate jive_gate;

typedef struct jive_resource_class jive_resource_class;
typedef struct jive_resource jive_resource;

struct jive_cpureg;
struct jive_regcls;
struct jive_region;
struct jive_graph;
struct jive_node;

/**
        \defgroup jive_type Types
        Types
        @{
*/

struct jive_type {
	const struct jive_type_class * class_;
};

struct jive_type_class {
	const struct jive_type_class * parent;
	
	/** \brief Give textual representation of type (for debugging) */
	char * (*get_label)(const jive_type * self);
	
	jive_input * (*create_input)(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand);
	
	jive_output * (*create_output)(const jive_type * self, struct jive_node * node, size_t index);
	
	jive_resource * (*create_resource)(const jive_type * self, struct jive_graph * graph);
	
	jive_gate * (*create_gate)(const jive_type * self, struct jive_graph * graph, const char * name);
	
	bool (*equals)(const jive_type * self, const jive_type * other);
	
	bool (*accepts)(const jive_type * self, const jive_type * other);
};

extern const struct jive_type_class JIVE_TYPE;

/* single type instance */
const jive_type jive_type_singleton;

static inline bool
jive_type_isinstance(const jive_type * self, const jive_type_class * class_)
{
	const jive_type_class * c = self->class_;
	while(c) {
		if (c == class_) return true;
		c = c->parent;
	}
	return false;
}

/* returned string dynamically using malloc */
static inline char *
jive_type_get_label(const jive_type * self)
{
	return self->class_->get_label(self);
}

static inline jive_input *
jive_type_create_input(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	return self->class_->create_input(self, node, index, initial_operand);
}

static inline jive_output *
jive_type_create_output(const jive_type * self, struct jive_node * node, size_t index)
{
	return self->class_->create_output(self, node, index);
}

static inline jive_resource *
jive_type_create_resource(const jive_type * self, struct jive_graph * graph)
{
	return self->class_->create_resource(self, graph);
}

static inline jive_gate *
jive_type_create_gate(const jive_type * self, struct jive_graph * graph, const char * name)
{
	return self->class_->create_gate(self, graph, name);
}

static inline bool
jive_type_equals(const jive_type * self, const jive_type * other)
{
	return self->class_->equals(self, other);
}

static inline bool
jive_type_accepts(const jive_type * self, const jive_type * other)
{
	return self->class_->accepts(self, other);
}

/**
        \defgroup jive_input Inputs
        Inputs
        @{
*/

struct jive_input {
	const struct jive_input_class * class_;
	
	struct jive_node * node;
	size_t index;
	jive_output * origin;
	jive_gate * gate;
	jive_resource * resource;
	
	struct {
		jive_input * prev;
		jive_input * next;
	} output_users_list;
	
	struct {
		jive_input * prev;
		jive_input * next;
	} gate_inputs_list;
	
	struct {
		jive_input * prev;
		jive_input * next;
	} resource_input_list;
};

struct jive_input_class {
	const struct jive_input_class * parent;
	
	void (*fini)(jive_input * self);
	
	/** \brief Give textual representation of type (for debugging) */
	char * (*get_label)(const jive_input * self);
	
	/** \brief Retrieve type of input */
	const jive_type * (*get_type)(const jive_input * self);
	
	/** \brief Retrieve resource constraint of input */
	jive_resource * (*get_constraint)(const jive_input * self);
};

extern const jive_input_class JIVE_INPUT;

static inline bool
jive_input_isinstance(const jive_input * self, const jive_input_class * class_)
{
	const jive_input_class * c = self->class_;
	while(c) {
		if (c == class_) return true;
		c = c->parent;
	}
	return false;
}

void
jive_input_divert_origin(jive_input * self, jive_output * new_origin);

void
jive_input_swap(jive_input * self, jive_input * other);

/* returned string dynamically using malloc */
static inline char *
jive_input_get_label(const jive_input * self)
{
	return self->class_->get_label(self);
}

static inline const jive_type *
jive_input_get_type(const jive_input * self)
{
	return self->class_->get_type(self);
}

static inline jive_resource *
jive_input_get_constraint(const jive_input * self)
{
	return self->class_->get_constraint(self);
}

void
jive_input_destroy(jive_input * self);

/**	@}	*/

/**
        \defgroup jive_output Outputs
        Outputs
        @{
*/

struct jive_output {
	const struct jive_output_class * class_;
	
	struct jive_node * node;
	size_t index;
	struct {
		jive_input * first;
		jive_input * last;
	} users;
	
	jive_gate * gate;
	struct {
		jive_output * prev;
		jive_output * next;
	} gate_outputs_list;
	
	jive_resource * resource;
	struct {
		jive_output * prev;
		jive_output * next;
	} resource_output_list;
};

struct jive_output_class {
	const struct jive_output_class * parent;
	
	void (*fini)(jive_output * self);
	
	/** \brief Give textual representation of type (for debugging) */
	char * (*get_label)(const jive_output * self);
	
	/** \brief Retrieve type of output */
	const jive_type * (*get_type)(const jive_output * self);
	
	/** \brief Retrieve resource constraint of output */
	jive_resource * (*get_constraint)(const jive_output * self);
};

extern const struct jive_output_class JIVE_OUTPUT;

static inline bool
jive_output_isinstance(const jive_output * self, const jive_output_class * class_)
{
	const jive_output_class * c = self->class_;
	while(c) {
		if (c == class_) return true;
		c = c->parent;
	}
	return false;
}

/* returned string dynamically using malloc */
static inline char *
jive_output_get_label(const jive_output * self)
{
	return self->class_->get_label(self);
}

static inline const jive_type *
jive_output_get_type(const jive_output * self)
{
	return self->class_->get_type(self);
}

static inline jive_resource *
jive_output_get_constraint(const jive_output * self)
{
	return self->class_->get_constraint(self);
}

void
jive_output_replace(jive_output * self, jive_output * other);

void
jive_output_destroy(jive_output * self);

/**	@}	*/

/**
        \defgroup jive_gate Gates
        Gates
        @{
*/

struct jive_gate {
	const struct jive_gate_class * class_;
	
	struct jive_graph * graph;
	char * name;
	struct {
		jive_input * first;
		jive_input * last;
	} inputs;
	struct {
		jive_output * first;
		jive_output * last;
	} outputs;
	bool may_spill;
	jive_resource * resource;
	void * interference; /* TODO: data type */
	
	struct {
		jive_gate * prev;
		jive_gate * next;
	} resource_gate_list;
	
	struct {
		jive_gate * prev;
		jive_gate * next;
	} graph_gate_list;
};

struct jive_gate_class {
	const struct jive_gate_class * parent;
	
	void (*fini)(jive_gate * self);
	
	char * (*get_label)(const jive_gate * self);
	
	const jive_type * (*get_type)(const jive_gate * self);
};

extern const struct jive_gate_class JIVE_GATE;

static inline char *
jive_gate_get_label(const jive_gate * self)
{
	return self->class_->get_label(self);
}

static inline const jive_type *
jive_gate_get_type(const jive_gate * self)
{
	return self->class_->get_type(self);
}

/**	@}	*/

/**
        \defgroup jive_resource Resources
        Resources
        @{
*/

struct jive_resource {
	const struct jive_resource_class * class_;
	
	struct jive_graph * graph;
	
	struct {
		jive_input * first;
		jive_input * last;
	} inputs;
	
	struct {
		jive_output * first;
		jive_output * last;
	} outputs;
	
	struct {
		jive_gate * first;
		jive_gate * last;
	} gates;
	
	jive_node_interaction node_interaction;
	jive_resource_interference_hash interference;
	struct jive_region * hovering_region;
	
	struct {
		jive_resource * prev;
		jive_resource * next;
	} graph_resource_list;
};

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
};

extern const struct jive_resource_class JIVE_RESOURCE;

bool
jive_resource_conflicts_with(const jive_resource * self, const jive_resource * other);

bool
jive_resource_may_spill(const jive_resource * self);

void
jive_resource_set_hovering_region(jive_resource * self, struct jive_region * region);

static inline bool
jive_resource_used(const jive_resource * self)
{
	return (self->inputs.first || self->outputs.first || self->gates.first);
}

static inline const jive_type *
jive_resource_get_type(const jive_resource * self)
{
	return self->class_->get_type(self);
}

static inline bool
jive_resource_can_merge(const jive_resource * self, const jive_resource * other)
{
	return self->class_->can_merge(self, other);
}

static inline void
jive_resource_merge(jive_resource * self, jive_resource * other)
{
	self->class_->merge(self, other);
}

static inline const struct jive_cpureg *
jive_resource_get_cpureg(const jive_resource * self)
{
	return self->class_->get_cpureg(self);
}

static inline const struct jive_regcls *
jive_resource_get_regcls(const jive_resource * self)
{
	return self->class_->get_regcls(self);
}

static inline const struct jive_regcls *
jive_resource_get_real_regcls(const jive_resource * self)
{
	return self->class_->get_real_regcls(self);
}

void
jive_resource_destroy(jive_resource * self);

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

/**	@}	*/

/**	@}	*/

#endif
