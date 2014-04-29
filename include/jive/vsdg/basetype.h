/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_BASETYPE_H
#define JIVE_VSDG_BASETYPE_H

#include <stdbool.h>
#include <stdlib.h>

#include <jive/common.h>
#include <jive/vsdg/gate-interference.h>

typedef struct jive_type jive_type;
typedef struct jive_type_class jive_type_class;

class jive_gate;
class jive_input;
class jive_output;

struct jive_buffer;
struct jive_cpureg;
struct jive_graph;
struct jive_node;
struct jive_regcls;
struct jive_region;
struct jive_resource_class;
struct jive_ssavar;
struct jive_variable;

/**
        \defgroup jive_type Types
        Types
        @{
*/

struct jive_type_class {
	const struct jive_type_class * parent;
	const char * name;

	void (*fini)(jive_type* self);
	
	/** \brief Give textual representation of type (for debugging) */
	void (*get_label)(const jive_type * self, struct jive_buffer * buffer);
	
	jive_input * (*create_input)(const jive_type * self, struct jive_node * node, size_t index,
		jive_output * initial_operand);
	
	jive_output * (*create_output)(const jive_type * self, struct jive_node * node, size_t index);
	
	jive_gate * (*create_gate)(const jive_type * self, struct jive_graph * graph, const char * name);
	
	bool (*equals)(const jive_type * self, const jive_type * other);
	
	/** \brief Create dynamically allocated copy of type */
	jive_type * (*copy)(const jive_type * self);
};

extern const struct jive_type_class JIVE_TYPE;

class jive_type {
public:
	virtual ~jive_type() noexcept;

protected:
	jive_type(const jive_type_class * class__) noexcept;

public:
	const struct jive_type_class * class_;
};

JIVE_EXPORTED_INLINE bool
jive_type_isinstance(const jive_type * self, const jive_type_class * class_)
{
	const jive_type_class * c = self->class_;
	while(c) {
		if (c == class_) return true;
		c = c->parent;
	}
	return false;
}

JIVE_EXPORTED_INLINE void
jive_type_get_label(const jive_type * self, struct jive_buffer * buffer)
{
	self->class_->get_label(self, buffer);
}

struct jive_input *
jive_type_create_input(const jive_type * self, struct jive_node * node, size_t index,
	jive_output * initial_operand);

JIVE_EXPORTED_INLINE jive_output *
jive_type_create_output(const jive_type * self, struct jive_node * node, size_t index)
{
	return self->class_->create_output(self, node, index);
}

JIVE_EXPORTED_INLINE jive_gate *
jive_type_create_gate(const jive_type * self, struct jive_graph * graph, const char * name)
{
	return self->class_->create_gate(self, graph, name);
}

JIVE_EXPORTED_INLINE bool
jive_type_equals(const jive_type * self, const jive_type * other)
{
	return (self == other) || self->class_->equals(self, other);
}

JIVE_EXPORTED_INLINE jive_type *
jive_type_copy(const jive_type * self)
{
	return self->class_->copy(self);
}

JIVE_EXPORTED_INLINE void
jive_type_fini(jive_type * self)
{
	self->class_->fini(self);
}

void
jive_type_destroy(struct jive_type * self);

/**
        \defgroup jive_input Inputs
        Inputs
        @{
*/

class jive_input {
public:
	virtual ~jive_input() noexcept;

protected:
	jive_input(struct jive_node * node, size_t index, jive_output * origin);

public:
	virtual const jive_type & type() const noexcept = 0;

	virtual void label(jive_buffer & buffer) const;
	
	struct jive_node * node;
	size_t index;
	
	jive_output * origin;
	struct {
		jive_input * prev;
		jive_input * next;
	} output_users_list;
	
	jive_gate * gate;
	struct {
		jive_input * prev;
		jive_input * next;
	} gate_inputs_list;
	
	struct jive_ssavar * ssavar;
	struct {
		jive_input * prev;
		jive_input * next;
	} ssavar_input_list;

	struct {
		struct jive_region_hull_entry * first;
		struct jive_region_hull_entry * last;
	} hull;
	
	const struct jive_resource_class * required_rescls;
};

void
jive_input_divert_origin(jive_input * self, jive_output * new_origin);

void
jive_input_swap(jive_input * self, jive_input * other);

JIVE_EXPORTED_INLINE void
jive_input_get_label(const jive_input * self, struct jive_buffer * buffer)
{
	self->label(*buffer);
}

JIVE_EXPORTED_INLINE const jive_type *
jive_input_get_type(const jive_input * self)
{
	return &self->type();
}

struct jive_variable *
jive_input_get_constraint(const jive_input * self);

void
jive_input_unassign_ssavar(jive_input * self);

struct jive_ssavar *
jive_input_auto_assign_variable(jive_input * self);

struct jive_ssavar *
jive_input_auto_merge_variable(jive_input * self);

void
jive_input_destroy(jive_input * self);

JIVE_EXPORTED_INLINE jive_output *
jive_input_origin(const jive_input * input)
{
	return input->origin;
}

JIVE_EXPORTED_INLINE struct jive_node *
jive_input_node(const jive_input * input)
{
	return input->node;
}

/**	@}	*/

/**
        \defgroup jive_output Outputs
        Outputs
        @{
*/

class jive_output {
public:
	virtual ~jive_output() noexcept;

protected:
	jive_output(struct jive_node * node, size_t index);

public:
	virtual const jive_type & type() const noexcept = 0;

	virtual void label(jive_buffer & buffer) const;

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
	
	struct jive_ssavar * ssavar;
	
	struct {
		struct jive_ssavar * first;
		struct jive_ssavar * last;
	} originating_ssavars;
	
	const struct jive_resource_class * required_rescls;
};

JIVE_EXPORTED_INLINE bool
jive_output_has_no_user(const struct jive_output * self)
{
	return self->users.first == NULL;
}

JIVE_EXPORTED_INLINE bool
jive_output_has_single_user(const struct jive_output * self)
{
	return (self->users.first != NULL) && (self->users.first == self->users.last);
}

JIVE_EXPORTED_INLINE void
jive_output_get_label(const jive_output * self, struct jive_buffer * buffer)
{
	self->label(*buffer);
}

JIVE_EXPORTED_INLINE const jive_type *
jive_output_get_type(const jive_output * self)
{
	return &self->type();
}

struct jive_variable *
jive_output_get_constraint(const jive_output * self);

void
jive_output_replace(jive_output * self, jive_output * other);

struct jive_ssavar *
jive_output_auto_assign_variable(jive_output * self);

struct jive_ssavar *
jive_output_auto_merge_variable(jive_output * self);

void
jive_output_destroy(jive_output * self);

JIVE_EXPORTED_INLINE struct jive_node *
jive_output_node(const jive_output * output)
{
	return output->node;
}

/**	@}	*/

/**
        \defgroup jive_gate Gates
        Gates
        @{
*/

class jive_gate {
public:
	virtual ~jive_gate() noexcept;

protected:
	jive_gate(struct jive_graph * graph, const char name[]);

public:
	virtual const jive_type & type() const noexcept = 0;

	virtual void label(jive_buffer & buffer) const;
	
	struct jive_graph * graph;
	struct {
		jive_gate * prev;
		jive_gate * next;
	} graph_gate_list;
	
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
	jive_gate_interference_hash interference;
	
	struct jive_variable * variable;
	struct {
		jive_gate * prev;
		jive_gate * next;
	} variable_gate_list;
	
	const struct jive_resource_class * required_rescls;
};

JIVE_EXPORTED_INLINE void
jive_gate_get_label(const jive_gate * self, struct jive_buffer * buffer)
{
	self->label(*buffer);
}

JIVE_EXPORTED_INLINE const jive_type *
jive_gate_get_type(const jive_gate * self)
{
	return &self->type();
}

struct jive_variable *
jive_gate_get_constraint(jive_gate * self);

JIVE_EXPORTED_INLINE jive_input *
jive_gate_create_input(const jive_gate * self, struct jive_node * node, size_t index,
	jive_output * initial_operand)
{
	jive_input * input = jive_type_create_input(jive_gate_get_type(self), node, index,
		initial_operand);
	input->required_rescls = self->required_rescls;
	return input;
}

JIVE_EXPORTED_INLINE jive_output *
jive_gate_create_output(const jive_gate * self, struct jive_node * node, size_t index)
{
	jive_output * output = jive_type_create_output(jive_gate_get_type(self), node, index);
	output->required_rescls = self->required_rescls;
	return output;
}

size_t
jive_gate_interferes_with(const jive_gate * self, const jive_gate * other);

void
jive_gate_merge(jive_gate * self, jive_gate * other);

void
jive_gate_split(jive_gate * self);

void
jive_gate_auto_merge_variable(jive_gate * self);

void
jive_gate_destroy(jive_gate * self);

/**	@}	*/

/**	@}	*/

void
jive_raise_type_error(const jive_type * self, const jive_type * other,
	struct jive_context * context);

#endif
