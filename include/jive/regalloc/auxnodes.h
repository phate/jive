/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_AUXNODES_H
#define JIVE_REGALLOC_AUXNODES_H

#include <jive/common.h>

struct jive_shaped_graph;

/* replace all "split" nodes with real instruction nodes */

void
jive_regalloc_auxnodes_replace(struct jive_shaped_graph * shaped_graph);

#endif
