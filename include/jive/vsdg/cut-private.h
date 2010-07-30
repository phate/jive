#ifndef JIVE_VSDG_CUT_PRIVATE_H
#define JIVE_VSDG_CUT_PRIVATE_H

#include <jive/vsdg/cut.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

static inline void
jive_cut_init(jive_cut * self, jive_region * region)
{
	self->region = region;
	self->region_cuts_list.prev = self->region_cuts_list.next = 0;
	self->nodes.first = self->nodes.last = 0;
}

void
jive_node_location_destroy(jive_node_location * self);

#endif
