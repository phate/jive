#ifndef JIVE_VSDG_REGION_PRIVATE_H
#define JIVE_VSDG_REGION_PRIVATE_H

#include <jive/vsdg/region.h>
#include <jive/vsdg/graph.h>

void
_jive_region_init(jive_region * self, jive_graph * graph, jive_region * parent);

void
_jive_region_fini(jive_region * self);

#endif
