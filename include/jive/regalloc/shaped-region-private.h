/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_SHAPED_REGION_PRIVATE_H
#define JIVE_REGALLOC_SHAPED_REGION_PRIVATE_H

#include <jive/regalloc/notifiers.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-region.h>
#include <jive/regalloc/xpoint-private.h>

static inline void
jive_shaped_region_add_active_top(jive_shaped_region * self, struct jive_shaped_ssavar * shaped_ssavar, size_t count)
{
	if (jive_region_varcut_ssavar_add(&self->active_top, shaped_ssavar, count) == 0) {
		jive_shaped_region_ssavar_notifier_slot_call(&self->shaped_graph->on_shaped_region_ssavar_add, self, shaped_ssavar);
	}
}

static inline void
jive_shaped_region_remove_active_top(jive_shaped_region * self, struct jive_shaped_ssavar * shaped_ssavar, size_t count)
{
	if (jive_region_varcut_ssavar_remove(&self->active_top, shaped_ssavar, count) == count) {
		jive_shaped_region_ssavar_notifier_slot_call(&self->shaped_graph->on_shaped_region_ssavar_remove, self, shaped_ssavar);
	}
}

#endif
