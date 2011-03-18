#ifndef JIVE_VSDG_REGION_SSAVAR_USE_PRIVATE_H
#define JIVE_VSDG_REGION_SSAVAR_USE_PRIVATE_H

#include <jive/vsdg/region-ssavar-use.h>

JIVE_DEFINE_HASH_TYPE(jive_region_ssavar_hash, jive_region_ssavar_use, struct jive_ssavar *, ssavar, hash_by_ssavar);
JIVE_DEFINE_HASH_TYPE(jive_ssavar_region_hash, jive_region_ssavar_use, struct jive_region *, region, hash_by_region);

#endif
