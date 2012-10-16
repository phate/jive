/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_STACKFRAME_H
#define JIVE_REGALLOC_STACKFRAME_H

struct jive_shaped_graph;

void
jive_regalloc_stackframe(struct jive_shaped_graph * shaped_graph);

void
jive_regalloc_relocate_stackslots(struct jive_shaped_graph * shaped_graph);

#endif
