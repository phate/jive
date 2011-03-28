#ifndef JIVE_REGALLOC_XPOINT_PRIVATE_H
#define JIVE_REGALLOC_XPOINT_PRIVATE_H

#include <jive/context.h>
#include <jive/regalloc/xpoint.h>

JIVE_DEFINE_HASH_TYPE(jive_node_xpoint_hash, jive_xpoint, struct jive_shaped_node *, shaped_node, node_hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_ssavar_xpoint_hash, jive_xpoint, struct jive_shaped_ssavar *, shaped_ssavar, ssavar_hash_chain);

jive_xpoint *
jive_xpoint_create(struct jive_shaped_node * shaped_node, struct jive_shaped_ssavar * shaped_ssavar);

void
jive_xpoint_destroy(jive_xpoint * self);

static inline void
jive_xpoint_put(jive_xpoint * self)
{
	if (self->before_count == 0 && self->after_count == 0)
		jive_xpoint_destroy(self);
}

JIVE_DEFINE_HASH_TYPE(jive_region_tpoint_hash, jive_tpoint, struct jive_shaped_region *, shaped_region, region_hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_ssavar_tpoint_hash, jive_tpoint, struct jive_shaped_ssavar *, shaped_ssavar, ssavar_hash_chain);

jive_tpoint *
jive_tpoint_create(struct jive_shaped_region * shaped_region, struct jive_shaped_ssavar * shaped_ssavar);

void
jive_tpoint_destroy(jive_tpoint * self);

static inline void
jive_tpoint_put(jive_tpoint * self)
{
	if (self->count == 0)
		jive_tpoint_destroy(self);
}

#endif
