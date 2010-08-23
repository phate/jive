#ifndef JIVE_ARCH_SUBROUTINE_H
#define JIVE_ARCH_SUBROUTINE_H

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/types.h>
#include <jive/context.h>

typedef struct jive_subroutine jive_subroutine;
typedef struct jive_subroutine_node jive_subroutine_node;
typedef struct jive_subroutine_class jive_subroutine_class;
typedef enum jive_argument_type jive_argument_type;

extern const jive_node_class JIVE_SUBROUTINE_NODE;

struct jive_subroutine_node {
	jive_node base;
	jive_subroutine * subroutine;
};

struct jive_subroutine {
	const jive_subroutine_class * class_;
	jive_subroutine_node * subroutine_node;
	struct jive_region * region;
	jive_node * enter;
	jive_node * leave;
};

struct jive_subroutine_class {
	void (*fini)(jive_subroutine * self);
	jive_output * (*get_parameter)(jive_subroutine * self, size_t index);
	jive_input * (*return_value)(jive_subroutine * self, jive_output * value);
};

static inline void
jive_subroutine_destroy(jive_subroutine * self)
{
	self->class_->fini(self);
	jive_context_free(self->subroutine_node->base.graph->context, self);
}

static inline jive_output *
jive_subroutine_get_parameter(jive_subroutine * self, size_t index)
{
	return self->class_->get_parameter(self, index);
}

static inline jive_input *
jive_subroutine_return_value(jive_subroutine * self, jive_output * value)
{
	return self->class_->return_value(self, value);
}

enum jive_argument_type {
	jive_argument_void = 0,
	jive_argument_pointer = 1,
	jive_argument_int = 2,
	jive_argument_long = 3
};

#endif
