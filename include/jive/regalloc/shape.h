/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_SHAPE_H
#define JIVE_REGALLOC_SHAPE_H

struct jive_graph;
struct jive_shaped_graph;

struct jive_shaped_graph *
jive_regalloc_shape(struct jive_graph * graph);

#endif
