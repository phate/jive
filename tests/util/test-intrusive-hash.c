/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <jive/util/intrusive-hash.h>

struct my_item {
	int key;
	int value;
	struct {
		my_item * prev;
		my_item * next;
	} hash_chain;
};

struct my_accessor {
	int get_key(const my_item * item) const noexcept { return item->key; }
	my_item * get_prev(const my_item * item) const noexcept { return item->hash_chain.prev; }
	my_item * get_next(const my_item * item) const noexcept { return item->hash_chain.next; }
	void set_prev(my_item * item, my_item * prev) const noexcept { item->hash_chain.prev = prev; }
	void set_next(my_item * item, my_item * next) const noexcept { item->hash_chain.next = next; }
};

typedef jive::detail::intrusive_hash<
	int,
	my_item,
	my_accessor> my_hash;

static int test_main(void)
{
	my_hash m;
	
	assert(m.find(42) == m.end());
	
	my_item i1 = {42, 0};
	m.insert(&i1);
	assert(&*m.find(42) == &i1);
	
	my_item i2 = {10, 0};
	m.insert(&i2);
	
	m.erase(&i1);
	assert(m.find(42) == m.end());
	m.insert(&i1);
	assert(&*m.find(42) == &i1);

	int seen_i1 = 0, seen_i2 = 0;
	for (const my_item & i : m) {
		assert((&i == &i1) || (&i == &i2));
		if (&i == &i1) {
			++ seen_i1;
		}
		if (&i == &i2) {
			++ seen_i2;
		}
	}
	assert(seen_i1 == 1);
	assert(seen_i2 == 1);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("util/test-intrusive-hash", test_main);
