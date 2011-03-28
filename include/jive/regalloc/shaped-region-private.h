#ifndef JIVE_REGALLOC_SHAPED_REGION_PRIVATE_H
#define JIVE_REGALLOC_SHAPED_REGION_PRIVATE_H

#include <jive/regalloc/xpoint-private.h>

static inline void
jive_shaped_region_add_active_top(jive_shaped_region * self, struct jive_shaped_ssavar * shaped_ssavar, size_t count)
{
	jive_tpoint * tpoint = jive_region_tpoint_hash_lookup(&shaped_ssavar->region_tpoints, self);
	if (!tpoint)
		tpoint = jive_tpoint_create(self, shaped_ssavar);
	tpoint->count += count;
}

static inline void
jive_shaped_region_remove_active_top(jive_shaped_region * self, struct jive_shaped_ssavar * shaped_ssavar, size_t count)
{
	jive_tpoint * tpoint = jive_region_tpoint_hash_lookup(&shaped_ssavar->region_tpoints, self);
	tpoint->count -= count;
	if (tpoint->count == 0)
		jive_tpoint_destroy(tpoint);
}

#endif
