#ifndef JIVE_RESOURCE_INTERFERENCE_H
#define JIVE_RESOURCE_INTERFERENCE_H

typedef struct jive_resource_interference jive_resource_interference;
typedef struct jive_resource_interference_part jive_resource_interference_part;
typedef struct jive_resource_interference_hash jive_resource_interference_hash;
typedef struct jive_resource_interference_hash_bucket jive_resource_interference_hash_bucket;

struct jive_resource_interference_hash_bucket {
	jive_resource_interference_part * first;
	jive_resource_interference_part * last;
};

struct jive_resource_interference_hash {
	size_t nitems, nbuckets;
	jive_resource_interference_hash_bucket * buckets;
};

#endif
