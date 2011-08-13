#ifndef JIVE_ARCH_SUBROUTINE_H
#define JIVE_ARCH_SUBROUTINE_H

#include <jive/context.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

struct jive_output;
struct jive_region;

extern const jive_node_class JIVE_SUBROUTINE_ENTER_NODE;
extern const jive_node_class JIVE_SUBROUTINE_LEAVE_NODE;
extern const jive_node_class JIVE_SUBROUTINE_NODE;

typedef struct jive_subroutine jive_subroutine;

typedef struct jive_subroutine_node_attrs jive_subroutine_node_attrs;

typedef struct jive_subroutine_enter_node jive_subroutine_enter_node;
typedef struct jive_subroutine_leave_node jive_subroutine_leave_node;
typedef struct jive_subroutine_node jive_subroutine_node;

typedef struct jive_subroutine_class jive_subroutine_class;
typedef enum jive_argument_type jive_argument_type;

struct jive_subroutine_node_attrs {
	jive_node_attrs base;
	jive_subroutine * subroutine;
};

struct jive_subroutine_node {
	jive_node base;
	jive_subroutine_node_attrs attrs;
};

struct jive_subroutine_enter_node {
	jive_node base;
	jive_subroutine_node_attrs attrs;
};

struct jive_subroutine_leave_node {
	jive_node base;
	jive_subroutine_node_attrs attrs;
};

typedef struct jive_subroutine_passthrough jive_subroutine_passthrough;

struct jive_subroutine_passthrough {
	jive_gate * gate;
	jive_output * output;
	jive_input * input;
};

JIVE_EXPORTED_INLINE jive_subroutine_node *
jive_subroutine_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_SUBROUTINE_NODE)
		return (jive_subroutine_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_subroutine_enter_node *
jive_subroutine_enter_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_SUBROUTINE_ENTER_NODE)
		return (jive_subroutine_enter_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_subroutine_leave_node *
jive_subroutine_leave_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_SUBROUTINE_LEAVE_NODE)
		return (jive_subroutine_leave_node *) node;
	else
		return NULL;
}

struct jive_subroutine {
	const jive_subroutine_class * class_;
	jive_context * context;
	jive_subroutine_node * subroutine_node;
	jive_subroutine_enter_node * enter;
	jive_subroutine_leave_node * leave;
	struct jive_region * region;
	
	size_t nparameters;
	jive_gate ** parameters;
	
	size_t nreturns;
	jive_gate ** returns;
};

struct jive_subroutine_class {
	void (*fini)(jive_subroutine * self);
	jive_output * (*value_parameter)(jive_subroutine * self, size_t index);
	jive_input * (*value_return)(jive_subroutine * self, size_t index, jive_output * value);
	jive_subroutine * (*copy)(const jive_subroutine * self,
		jive_node * new_enter_node, jive_node * new_leave_node);
};

void
jive_subroutine_destroy(jive_subroutine * self);

JIVE_EXPORTED_INLINE jive_output *
jive_subroutine_value_parameter(jive_subroutine * self, size_t index)
{
	return self->class_->value_parameter(self, index);
}

JIVE_EXPORTED_INLINE jive_input *
jive_subroutine_value_return(jive_subroutine * self, size_t index, jive_output * value)
{
	return self->class_->value_return(self, index, value);
}

/* FIXME: these are quite C-specific, so really do not belong here */

enum jive_argument_type {
	jive_argument_void = 0,
	jive_argument_pointer = 1,
	jive_argument_int = 2,
	jive_argument_long = 3
};

#endif
