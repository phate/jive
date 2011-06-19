#ifndef JIVE_REGALLOC_SHAPED_REGION_PRIVATE_H
#define JIVE_REGALLOC_SHAPED_REGION_PRIVATE_H

#include <jive/regalloc/xpoint-private.h>

static inline void
jive_shaped_region_add_active_top(jive_shaped_region * self, struct jive_shaped_ssavar * shaped_ssavar, size_t count)
{
	jive_cutvar_xpoint * xpoint = jive_cutvar_xpoint_hash_byregion_lookup(&shaped_ssavar->region_xpoints, self);
	if (!xpoint)
		xpoint = jive_cutvar_xpoint_create(self, shaped_ssavar);
	xpoint->count += count;
}

static inline void
jive_shaped_region_remove_active_top(jive_shaped_region * self, struct jive_shaped_ssavar * shaped_ssavar, size_t count)
{
	jive_cutvar_xpoint * xpoint = jive_cutvar_xpoint_hash_byregion_lookup(&shaped_ssavar->region_xpoints, self);
	xpoint->count -= count;
	if (xpoint->count == 0)
		jive_cutvar_xpoint_destroy(xpoint);
}

#endif
