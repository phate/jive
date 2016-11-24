/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_EQUIVALENCE_H
#define JIVE_VSDG_EQUIVALENCE_H

#include <jive/vsdg/graph.h>

bool
jive_graphs_equivalent(
	jive_graph * graph1, jive_graph * graph2,
	size_t ncheck, jive::node * const check1[], jive::node * const check2[],
	size_t nassumed, jive::node * const ass1[], jive::node * const ass2[]);

#endif
