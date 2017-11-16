/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_RVSDG_EQUIVALENCE_H
#define JIVE_RVSDG_EQUIVALENCE_H

#include <jive/rvsdg/graph.h>

bool
jive_graphs_equivalent(
	jive::graph * graph1, jive::graph * graph2,
	size_t ncheck, jive::node * const check1[], jive::node * const check2[],
	size_t nassumed, jive::node * const ass1[], jive::node * const ass2[]);

#endif
