/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_CLG_NODE_H
#define JIVE_FRONTEND_CLG_NODE_H

#include <stdbool.h>
#include <stddef.h>

struct jive_buffer;
struct jive_function_type;

typedef struct jive_clg_node jive_clg_node;
typedef struct jive_clg_node_class jive_clg_node_class;

struct jive_clg_node {
	const struct jive_clg_node_class * class_;

	struct jive_clg * clg;

	size_t ncalls;
	struct jive_clg_node ** calls;

	struct {
		struct jive_clg_node * prev;
		struct jive_clg_node * next;
	} clg_node_list;

	char * name;
	struct jive_cfg * cfg;
};

extern const jive_clg_node_class JIVE_CLG_NODE;

struct jive_clg_node_class {
	const struct jive_clg_node_class * parent;
	const char * name;

	void (*fini)(jive_clg_node * self);

	void (*get_label)(const struct jive_clg_node * self, struct jive_buffer * buffer);

	jive_clg_node * (*create)(struct jive_clg * clg, const char * name);
};

static inline bool
jive_clg_node_isinstance(const jive_clg_node * self, const jive_clg_node_class * class_)
{
	const jive_clg_node_class * c = self->class_;
	while (c) {
		if (c == class_)
			return true;
		c = c->parent;
	}
	return false;
}

static inline void
jive_clg_node_get_label(const jive_clg_node * self, struct jive_buffer * buffer)
{
	self->class_->get_label(self, buffer);
}

struct jive_clg_node *
jive_clg_node_create(struct jive_clg * clg, const char * name);

void
jive_clg_node_add_call(struct jive_clg_node * self, struct jive_clg_node * callee);

void
jive_clg_node_destroy(struct jive_clg_node * self);

#endif
