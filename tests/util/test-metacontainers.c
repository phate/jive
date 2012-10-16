/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <jive/internal/metacontainers.h>
#include <jive/util/list.h>

void test_list(void)
{
	struct my_struct {
		struct {
			struct my_struct * prev, * next;
		} anchor;
	};
	struct my_struct_list {
		struct my_struct * first, * last;
	};
	
	struct my_struct_list list_head = {0,0};
	struct my_struct e1, e2, e3, e4, e5;
	
	static struct my_struct * volatile e3_ptr;
	e3_ptr = &e3;
	
	JIVE_LIST_PUSH_FRONT(list_head, &e2, anchor);
	JIVE_LIST_PUSH_FRONT(list_head, &e1, anchor);
	JIVE_LIST_PUSH_BACK(list_head, &e4, anchor);
	JIVE_LIST_INSERT(list_head, e3_ptr, &e4, anchor);
	JIVE_LIST_INSERT(list_head, (struct my_struct *)0, &e5, anchor);
	JIVE_LIST_REMOVE(list_head, &e3, anchor);
	
	assert(list_head.first == &e1 && list_head.last == &e5);
	assert(e1.anchor.prev == 0 && e1.anchor.next == &e2);
	assert(e2.anchor.prev == &e1 && e2.anchor.next == &e4);
	assert(e4.anchor.prev == &e2 && e4.anchor.next == &e5);
	assert(e5.anchor.prev == &e4 && e5.anchor.next == 0);
}

typedef struct int_set int_set;
DEFINE_SET_TYPE(int_set, int);

void test_set(jive_context * context)
{
	int_set set, set2;
	int_set_init(&set, context);
	
	assert( ! int_set_contains(&set, 42));
	
	assert( ! int_set_add(&set, 42));
	assert( ! int_set_add(&set, 41));
	assert( ! int_set_add(&set, 43));
	
	assert( int_set_add(&set, 41));
	
	assert(set.nitems == 3);
	assert(set.items[0] == 41);
	assert(set.items[1] == 42);
	assert(set.items[2] == 43);
	
	int_set_init(&set2, context);
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
	
	int_set_fini(&set);
	int_set_fini(&set2);
}

typedef struct int_multiset int_multiset;
DEFINE_MULTISET_TYPE(int_multiset, int);

void test_multiset(jive_context * context)
{
	int_multiset multiset;
	int_multiset_init(&multiset, context);
	
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
	
	int_multiset_fini(&multiset);
}

static inline int int_hash(int n) {return n;}

DEFINE_HASHMAP_TYPE(int_int_hashmap, int, int, int_hash);

void test_map(jive_context * context)
{
	struct int_int_hashmap map;
	int_int_hashmap_init(&map, context);
	
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
	
	int_int_hashmap_fini(&map);
}

static int test_main(void)
{
	jive_context * context = jive_context_create();
	test_set(context);
	test_multiset(context);
	test_map(context);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("util/test-metacontainers", test_main);
