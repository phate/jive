#include "test-registry.h"

#include <assert.h>
#include <jive/context.h>
#include <jive/util/hash.h>

typedef struct my_item my_item;

struct my_item {
	void * key;
	int value;
	struct {
		my_item * prev;
		my_item * next;
	} hash_chain;
};

JIVE_DECLARE_HASH_TYPE(my_hash, my_item, void *, key, hash_chain);
JIVE_DEFINE_HASH_TYPE(my_hash, my_item, void *, key, hash_chain);

typedef struct my_hash my_hash;
typedef struct my_hash_iterator my_hash_iterator;

static int test_main(void)
{
	jive_context * ctx = jive_context_create();
	
	my_hash hash;
	my_hash_init(&hash, ctx);
	
	assert(my_hash_lookup(&hash, (void *) 42) == 0);
	
	my_item i1 = {(void *)42, 0};
	my_hash_insert(&hash, &i1);
	assert(my_hash_lookup(&hash, (void *)42) == &i1);
	
	my_item i2 = {(void *)10, 0};
	my_hash_insert(&hash, &i2);
	
	my_hash_remove(&hash, &i1);
	assert(my_hash_lookup(&hash, (void *)42) == 0);
	my_hash_insert(&hash, &i1);
	assert(my_hash_lookup(&hash, (void *)42) == &i1);
	
	my_hash_iterator i;
	int seen_i1 = 0, seen_i2 = 0;
	JIVE_HASH_ITERATE(my_hash, hash, i) {
		assert((i.entry == &i1) || (i.entry == &i2));
		if (i.entry == &i1) seen_i1 ++;
		if (i.entry == &i2) seen_i2 ++;
	}
	
	assert(seen_i1 == 1);
	assert(seen_i2 == 1);
	
	my_hash_fini(&hash);
	
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("util/test-hash", test_main);
