#ifndef JIVE_VSDG_REGION_SSAVAR_USE_H
#define JIVE_VSDG_REGION_SSAVAR_USE_H

#include <stddef.h>

#include <jive/util/hash.h>

typedef struct jive_region_ssavar_use jive_region_ssavar_use;
typedef struct jive_region_ssavar_hash jive_region_ssavar_hash;
typedef struct jive_ssavar_region_hash jive_ssavar_region_hash;

struct jive_region;
struct jive_ssavar;

struct jive_region_ssavar_use {
	struct jive_region * region;
	struct {
		jive_region_ssavar_use * prev;
		jive_region_ssavar_use * next;
	} hash_by_region;
	
	struct jive_ssavar * ssavar;
	
	struct {
		jive_region_ssavar_use * prev;
		jive_region_ssavar_use * next;
	} hash_by_ssavar;
	
	size_t count;
};

JIVE_DECLARE_HASH_TYPE(jive_region_ssavar_hash, jive_region_ssavar_use, struct jive_ssavar *, ssavar, hash_by_ssavar);
JIVE_DECLARE_HASH_TYPE(jive_ssavar_region_hash, jive_region_ssavar_use, struct jive_region *, region, hash_by_region);

#endif
