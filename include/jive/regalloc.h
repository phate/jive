/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_H
#define JIVE_REGALLOC_H

struct jive_graph;
struct jive_shaped_graph;

struct jive_shaped_graph *
jive_regalloc(struct jive_graph * graph);

#endif
