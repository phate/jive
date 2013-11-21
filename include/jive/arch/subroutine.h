/*
 * Copyright 2010 2011 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_SUBROUTINE_H
#define JIVE_ARCH_SUBROUTINE_H

#include <jive/context.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

struct jive_instructionset;
struct jive_output;
struct jive_region;

struct jive_subroutine_enter_node;
struct jive_subroutine_leave_node;
struct jive_subroutine_node;

typedef struct jive_subroutine jive_subroutine;
struct jive_subroutine_builder;

struct jive_subroutine {
	struct jive_region * region;
	struct jive_subroutine_deprecated * old_subroutine_struct;
};

/**
	\brief Begin constructing a subroutine region
*/
jive_subroutine
jive_subroutine_begin(struct jive_graph * graph);

/**
	\brief End constructing a subroutine region
*/
struct jive_node *
jive_subroutine_end(jive_subroutine self);

/**
	\brief Get argument value
*/
struct jive_output *
jive_subroutine_simple_get_argument(
	jive_subroutine self,
	size_t index);

void
jive_subroutine_simple_set_result(
	jive_subroutine self,
	size_t index,
	struct jive_output * value);

typedef struct jive_subroutine_deprecated jive_subroutine_deprecated;

typedef struct jive_subroutine_node_attrs jive_subroutine_node_attrs;

typedef struct jive_subroutine_abi_class jive_subroutine_abi_class;
typedef struct jive_subroutine_class jive_subroutine_class;
typedef enum jive_argument_type jive_argument_type;

/* FIXME: these are quite C-specific, so really do not belong here */
enum jive_argument_type {
	jive_argument_void = 0,
	jive_argument_pointer = 1,
	jive_argument_int = 2,
	jive_argument_long = 3,
	jive_argument_float = 4
};

typedef struct jive_subroutine_passthrough jive_subroutine_passthrough;

struct jive_subroutine_passthrough {
	jive_gate * gate;
	jive_output * output;
	jive_input * input;
};

typedef struct jive_subroutine_late_transforms jive_subroutine_late_transforms;
typedef struct jive_value_split_factory jive_value_split_factory;

struct jive_value_split_factory {
	jive_output * (*split)(const jive_value_split_factory * self, jive_output * value);
};

struct jive_subroutine_late_transforms {
	void (*value_split)(const jive_subroutine_late_transforms * self,
		jive_output * value_in, jive_input * value_out,
		const jive_value_split_factory * enter_split, const jive_value_split_factory * leave_split);
};

typedef struct jive_subroutine_stackframe_info jive_subroutine_stackframe_info;
struct jive_subroutine_stackframe_info {
	/* lower bound of frame (relative to initial position of stack pointer) */
	ssize_t lower_bound;
	/* upper bound of frame (relative to initial position of stack pointer) */
	ssize_t upper_bound;
	/* offset of frame pointer to initial position of stack pointer */
	ssize_t frame_pointer_offset;
	/* offset of stack pointer to initial position of stack pointer */
	ssize_t stack_pointer_offset;
	/* size of argument area for calls */
	size_t call_area_size;
};

struct jive_subroutine_deprecated {
	const jive_subroutine_class * class_;
	const jive_subroutine_abi_class * abi_class;
	jive_context * context;
	struct jive_subroutine_node * subroutine_node;
	struct jive_subroutine_enter_node * enter;
	struct jive_subroutine_leave_node * leave;
	struct jive_region * region;
	
	size_t nparameters;
	jive_argument_type * parameter_types;
	jive_gate ** parameters;
	
	size_t nreturns;
	jive_argument_type * return_types;
	jive_gate ** returns;
	
	size_t npassthroughs;
	jive_subroutine_passthrough * passthroughs;
	
	jive_subroutine_stackframe_info frame;
};

struct jive_subroutine_class {
	void (*fini)(jive_subroutine_deprecated * self);
	jive_output * (*value_parameter)(jive_subroutine_deprecated * self, size_t index);
	jive_input * (*value_return)(jive_subroutine_deprecated * self, size_t index, jive_output * value);
};

struct jive_subroutine_abi_class {
	void (*prepare_stackframe)(jive_subroutine_deprecated * self, const jive_subroutine_late_transforms * xfrm);
	jive_input *(*add_fp_dependency)(const jive_subroutine_deprecated * self, jive_node * node);
	jive_input *(*add_sp_dependency)(const jive_subroutine_deprecated * self, jive_node * node);
	const struct jive_instructionset * instructionset;
};

void
jive_subroutine_destroy(jive_subroutine_deprecated * self);

JIVE_EXPORTED_INLINE jive_output *
jive_subroutine_value_parameter(jive_subroutine_deprecated * self, size_t index)
{
	return self->class_->value_parameter(self, index);
}

JIVE_EXPORTED_INLINE jive_input *
jive_subroutine_value_return(jive_subroutine_deprecated * self, size_t index, jive_output * value)
{
	return self->class_->value_return(self, index, value);
}

void
jive_subroutine_node_prepare_stackframe(
	struct jive_subroutine_node * self,
	const jive_subroutine_late_transforms * xfrm);

jive_input *
jive_subroutine_node_add_fp_dependency(
	const struct jive_subroutine_node * self,
	jive_node * node);

jive_input *
jive_subroutine_node_add_sp_dependency(
	const struct jive_subroutine_node * self,
	jive_node * node);

struct jive_subroutine_node *
jive_region_get_subroutine_node(const jive_region * region);

const struct jive_instructionset *
jive_region_get_instructionset(const jive_region * region);

jive_output *
jive_subroutine_node_get_sp(const struct jive_subroutine_node * self);

jive_output *
jive_subroutine_node_get_fp(const struct jive_subroutine_node * self);

jive_subroutine_stackframe_info *
jive_subroutine_node_get_stackframe_info(const struct jive_subroutine_node * self);

jive_output *
jive_subroutine_objdef(const jive_subroutine_deprecated * self);

#endif
