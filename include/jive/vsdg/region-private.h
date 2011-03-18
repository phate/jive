#ifndef JIVE_VSDG_REGION_PRIVATE_H
#define JIVE_VSDG_REGION_PRIVATE_H

#include <jive/vsdg/region.h>
#include <jive/vsdg/graph.h>

struct jive_ssavar;

void
_jive_region_init(jive_region * self, jive_graph * graph, jive_region * parent);

void
_jive_region_fini(jive_region * self);

void
_jive_region_prune_subregions(jive_region * self);

void
jive_region_add_used_ssavar(jive_region * self, struct jive_ssavar * ssavar);

void
jive_region_remove_used_ssavar(jive_region * self, struct jive_ssavar * ssavar);

#endif
