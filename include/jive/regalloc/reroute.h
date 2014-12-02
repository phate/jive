/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_REROUTE_H
#define JIVE_REGALLOC_REROUTE_H

struct jive_shaped_node;
struct jive_ssavar;

/**
	\brief Reroute variable through regions
	
	\param ssavar Variable to be rerouted
	\param node Point through which variable should be rerouted
	\return Rerouted value
	
	Reroute variable  through this point in shaped graph
	such that a split node can be inserted here; this means
	that this function guarantees that there will not be
	any users of this variable below that are in outer
	regions from the POV of this location.
*/
struct jive_ssavar *
jive_regalloc_reroute_at_point(struct jive_ssavar * ssavar, struct jive_shaped_node * node);

#endif
