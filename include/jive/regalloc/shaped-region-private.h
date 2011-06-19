#ifndef JIVE_REGALLOC_SHAPED_REGION_PRIVATE_H
#define JIVE_REGALLOC_SHAPED_REGION_PRIVATE_H

#include <jive/regalloc/xpoint-private.h>

static inline void
jive_shaped_region_add_active_top(jive_shaped_region * self, struct jive_shaped_ssavar * shaped_ssavar, size_t count)
{
	jive_region_varcut_ssavar_add(&self->active_top, shaped_ssavar, count);
}

static inline void
jive_shaped_region_remove_active_top(jive_shaped_region * self, struct jive_shaped_ssavar * shaped_ssavar, size_t count)
{
	jive_region_varcut_ssavar_remove(&self->active_top, shaped_ssavar, count);
}

#endif
