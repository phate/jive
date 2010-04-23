#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <jive/internal/metacontainers.h>

static inline void *
heap_realloc(void * ptr, size_t old_size, size_t new_size, int value)
{
	return realloc(ptr, new_size);
}

typedef struct int_set int_set;
DEFINE_SET_TYPE(int_set, int, heap_realloc);

void test_set()
{
	int_set set, set2;
	int_set_init(&set);
	
	assert( ! int_set_contains(&set, 42));
	
	assert( ! int_set_add(&set, 42));
	assert( ! int_set_add(&set, 41));
	assert( ! int_set_add(&set, 43));
	
	assert( int_set_add(&set, 41));
	
	assert(set.nitems == 3);
	assert(set.items[0] == 41);
	assert(set.items[1] == 42);
	assert(set.items[2] == 43);
	
	int_set_init(&set2);
	int_set_copy(&set2, &set);
	assert(set2.nitems == 3);
	assert(set2.items[0] == 41);
	assert(set2.items[1] == 42);
	assert(set2.items[2] == 43);
	
	assert( int_set_remove(&set, 42) );
	assert( !int_set_remove(&set, 44) );
	assert(set.nitems == 2);
	assert(set.items[0] == 41);
	assert(set.items[1] == 43);
}

typedef struct int_multiset int_multiset;
DEFINE_MULTISET_TYPE(int_multiset, int, heap_realloc);

void test_multiset()
{
	int_multiset multiset;
	int_multiset_init(&multiset);
	
	assert( int_multiset_contains(&multiset, 42) == 0 );
	
	assert( int_multiset_add(&multiset, 42) == 0 );
	assert( int_multiset_add(&multiset, 41) == 0 );
	assert( int_multiset_add(&multiset, 43) == 0 );
	
	assert( int_multiset_add(&multiset, 42) == 1 );
	
	assert(multiset.nitems == 3);
	assert(multiset.items[0].value == 41);
	assert(multiset.items[1].value == 42);
	assert(multiset.items[2].value == 43);
	
	assert( int_multiset_remove(&multiset, 42) == 2);
	assert(multiset.nitems == 3);
	assert(multiset.items[0].value == 41);
	assert(multiset.items[1].value == 42);
	assert(multiset.items[2].value == 43);
	
	assert( int_multiset_add(&multiset, 42) == 1 );
	assert( int_multiset_remove_all(&multiset, 42) == 2);
	assert(multiset.nitems == 2);
	assert(multiset.items[0].value == 41);
	assert(multiset.items[1].value == 43);
}

static inline int int_hash(int n) {return n;}
static inline void * int_int_hashmap_alloc(size_t space, int key, int value) {return malloc(space);}

DEFINE_HASHMAP_TYPE(int_int_hashmap, int, int, int_hash, int_int_hashmap_alloc);

void test_map()
{
	struct int_int_hashmap map;
	int_int_hashmap_init(&map);
	
	assert(!int_int_hashmap_lookup(&map, 0));
	
	assert(map.nitems == 0);
	
	int_int_hashmap_set(&map, 0, 42);
	assert(int_int_hashmap_lookup(&map, 0));
	assert(int_int_hashmap_lookup(&map, 0)->value == 42);
	assert(map.nitems == 1);
	
	int_int_hashmap_set(&map, 0, 43);
	assert(map.nitems == 1);
	
	bool found = int_int_hashmap_remove(&map, 0);
	assert(found);
	assert(map.nitems == 0);
	assert(int_int_hashmap_lookup(&map, 0) == 0);
	
	assert(int_int_hashmap_remove(&map, 0) == false);
}

int main()
{
	test_set();
	test_multiset();
	return 0;
}
