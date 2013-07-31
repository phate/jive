/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_COLLECTOR_H
#define JIVE_COLLECTOR_H

struct jive_context;

typedef struct jive_collector jive_collector;

struct jive_collector *
jive_collector_create(struct jive_context * context);

void
jive_collector_register(struct jive_collector * self, void * ptr, void * fini);

void
jive_collector_reclaim(struct jive_collector * self);

void
jive_collector_destroy(struct jive_collector * collector);

#endif
