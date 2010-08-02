#ifndef JIVE_REGCLS_COUNT_H
#define JIVE_REGCLS_COUNT_H

#include <jive/context.h>

typedef struct jive_regcls_count jive_regcls_count;
typedef struct jive_regcls_count_item jive_regcls_count_item;
typedef struct jive_regcls_count_bucket jive_regcls_count_bucket;

struct jive_regcls;

struct jive_regcls_count_item {
	const struct jive_regcls * regcls;
	size_t count;
	struct {
		jive_regcls_count_item * prev;
		jive_regcls_count_item * next;
	} chain;
};

struct jive_regcls_count_bucket {
	jive_regcls_count_item * first;
	jive_regcls_count_item * last;
};

struct jive_regcls_count {
	size_t nitems, nbuckets;
	jive_regcls_count_bucket * buckets;
};

#endif
