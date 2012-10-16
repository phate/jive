/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_TRAVERSER_H
#define JIVE_VSDG_TRAVERSER_H

#include <stdbool.h>
#include <stdlib.h>

struct jive_graph;
struct jive_node;
struct jive_region;

typedef struct jive_traverser jive_traverser;
typedef struct jive_traverser_class jive_traverser_class;

void
jive_traverser_destroy(jive_traverser * self);

struct jive_node *
jive_traverser_next(jive_traverser * self);

jive_traverser *
jive_topdown_traverser_create(struct jive_graph * graph);

jive_traverser *
jive_bottomup_traverser_create(struct jive_graph * graph);

jive_traverser *
jive_bottomup_revisit_traverser_create(struct jive_graph * graph);

jive_traverser *
jive_upward_cone_traverser_create(struct jive_node * node);

typedef struct jive_bottomup_region_traverser jive_bottomup_region_traverser;

void
jive_bottomup_region_traverser_destroy(jive_bottomup_region_traverser * self);

jive_traverser *
jive_bottomup_region_traverser_get_node_traverser(jive_bottomup_region_traverser * self, struct jive_region * region);

jive_bottomup_region_traverser *
jive_bottomup_region_traverser_create(struct jive_graph * graph);

void
jive_bottomup_region_traverser_pass(jive_bottomup_region_traverser * self, struct jive_node * node);

#endif
