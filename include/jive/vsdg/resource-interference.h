#ifndef JIVE_RESOURCE_INTERFERENCE_H
#define JIVE_RESOURCE_INTERFERENCE_H

#include <jive/util/hash.h>

typedef struct jive_resource_interference jive_resource_interference;
typedef struct jive_resource_interference_part jive_resource_interference_part;
typedef struct jive_resource_interference_hash jive_resource_interference_hash;

JIVE_DECLARE_HASH_TYPE(jive_resource_interference_hash, struct jive_resource_interference_part, struct jive_resource *, resource, chain);

#endif
