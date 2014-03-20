/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_CLG_H
#define JIVE_FRONTEND_CLG_H

struct jive_buffer;

typedef struct jive_clg jive_clg;

struct jive_clg {
	struct jive_context * context;

	struct {
		struct jive_clg_node * first;
		struct jive_clg_node * last;
	} nodes;
};

jive_clg *
jive_clg_create(struct jive_context * context);

void
jive_clg_convert_dot(const struct jive_clg * self, struct jive_buffer * buffer);

void
jive_clg_view(const struct jive_clg * self);

void
jive_clg_destroy(struct jive_clg * self);

#endif
