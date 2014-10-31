/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/resource.h>

#include <jive/context.h>
#include <jive/internal/compiler.h>
#include <jive/util/list.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/resource-private.h>

static inline size_t
jive_ptr_hash(const void * ptr)
{
	/* FIXME: hm, ideally I would like to "rotate" instead of "shifting"... */
	size_t hash = (size_t) ptr;
	hash ^= (hash >> 20) ^ (hash >> 12);
	return hash ^ (hash >> 7) ^ (hash >> 4);
}

const jive_resource_class *
jive_resource_class_union(const jive_resource_class * self, const jive_resource_class * other)
{
	for(;;) {
		if (self == other) return self;
		if (self->depth > other->depth)
			self = self->parent;
		else
			other = other->parent;
	}
}

const jive_resource_class *
jive_resource_class_intersection(const jive_resource_class * self, const jive_resource_class * other)
{
	const jive_resource_class * u = jive_resource_class_union(self, other);
	if (u == self) return other;
	else if (u == other) return self;
	else return 0;
}

jive::gate *
jive_resource_class_create_gate(const jive_resource_class * self, jive_graph * graph, const char * name)
{
	const jive::base::type * type = jive_resource_class_get_type(self);
	jive::gate * gate = type->create_gate(graph, name);
	gate->required_rescls = self;
	return gate;
}

const jive_resource_class *
jive_resource_class_relax(const jive_resource_class * self)
{
	/* hopefully this function is transitionary --
	currently everything that is needed is the
	class directly below the root */
	while (self->parent && !jive_resource_class_is_abstract(self->parent))
		self = self->parent;
	return self;
}

static const jive_resource_class_demotion no_demotion[] = {{NULL, NULL}};

const jive_resource_class_class JIVE_ABSTRACT_RESOURCE = {
	parent : 0,
	name : "root",
	is_abstract : true
};

const jive_resource_class jive_root_resource_class = {
	class_ : &JIVE_ABSTRACT_RESOURCE,
	name : "root",
	limit : 0,
	names : NULL,
	parent : 0,
	depth : 0,
	priority : jive_resource_class_priority_lowest,
	demotions : no_demotion,
	type : NULL
};

void
jive_resource_class_count_clear(jive_resource_class_count * self)
{
	jive_resource_class_count_item * item, * next_item;
	JIVE_LIST_ITERATE_SAFE(self->items, item, next_item, item_list) {
		size_t index = jive_ptr_hash(item->resource_class) & self->mask;
		JIVE_LIST_REMOVE(self->buckets[index], item, hash_chain);
		JIVE_LIST_REMOVE(self->items, item, item_list);
		delete item;
	}
	self->nitems = 0;
}

static jive_resource_class_count_item *
jive_resource_class_count_lookup_item(const jive_resource_class_count * self, const jive_resource_class * resource_class)
{
	if (!self->nbuckets)
		return 0;
	size_t index = jive_ptr_hash(resource_class) & self->mask;
	jive_resource_class_count_item * item;
	
	JIVE_LIST_ITERATE(self->buckets[index], item, item_list) {
		if (item->resource_class == resource_class)
			return item;
	}
	
	return 0;
}

size_t
jive_resource_class_count_get(const jive_resource_class_count * self, const struct jive_resource_class * resource_class)
{
	jive_resource_class_count_item * item = jive_resource_class_count_lookup_item(self, resource_class);
	if (item != 0)
		return item->count;
	else
		return 0;
}

static void
rehash(jive_resource_class_count * self)
{
	size_t new_nbuckets = self->nbuckets * 2;
	if (!new_nbuckets)
		new_nbuckets = 4;

	self->buckets.resize(new_nbuckets);

	size_t n;
	for(n=0; n<new_nbuckets; n++)
		self->buckets[n].first = self->buckets[n].last = 0;
	
	self->nbuckets = new_nbuckets;
	self->mask = new_nbuckets - 1;
	
	jive_resource_class_count_item * item;
	JIVE_LIST_ITERATE(self->items, item, item_list) {
		size_t index = jive_ptr_hash(item->resource_class) & self->mask;
		JIVE_LIST_PUSH_BACK(self->buckets[index], item, hash_chain);
	}
}

static size_t
jive_resource_class_count_add_single(jive_resource_class_count * self, const jive_resource_class * resource_class, size_t count)
{
	jive_resource_class_count_item * item = jive_resource_class_count_lookup_item(self, resource_class);
	
	if (likely(item != 0))
		return item->count += count;
	
	self->nitems ++;
	if (self->nitems >= self->nbuckets)
		rehash(self);
	
	item = new jive_resource_class_count_item;
	size_t index = jive_ptr_hash(resource_class) & self->mask;
	item->resource_class = resource_class;
	item->count = count;
	JIVE_LIST_PUSH_BACK(self->buckets[index], item, hash_chain);
	JIVE_LIST_PUSH_BACK(self->items, item, item_list);
	
	return item->count;
}

void
jive_resource_class_count_max(jive_resource_class_count * self, const struct jive_resource_class * resource_class, size_t count)
{
	jive_resource_class_count_item * item = jive_resource_class_count_lookup_item(self, resource_class);
	if (likely(item != 0)) {
		if (item->count < count) item->count = count;
		return;
	}
	self->nitems ++;
	if (self->nitems >= self->nbuckets)
		rehash(self);
	
	item = new jive_resource_class_count_item;
	size_t index = jive_ptr_hash(resource_class) & self->mask;
	item->resource_class = resource_class;
	item->count = count;
	JIVE_LIST_PUSH_BACK(self->buckets[index], item, hash_chain);
	JIVE_LIST_PUSH_BACK(self->items, item, item_list);
}

static void
jive_resource_class_count_sub_single(jive_resource_class_count * self, const jive_resource_class * resource_class)
{
	jive_resource_class_count_item * item = jive_resource_class_count_lookup_item(self, resource_class);
	item->count --;
	if (item->count == 0) {
		size_t index = jive_ptr_hash(resource_class) & self->mask;
		JIVE_LIST_REMOVE(self->buckets[index], item, hash_chain);
		JIVE_LIST_REMOVE(self->items, item, item_list);
		delete item;
		self->nitems --;
	}
}

const jive_resource_class *
jive_resource_class_count_add(jive_resource_class_count * self, const jive_resource_class * resource_class)
{
	const jive_resource_class * overflow = 0;
	while(resource_class) {
		size_t count = jive_resource_class_count_add_single(self, resource_class, 1);
		if (count > resource_class->limit && resource_class->limit && ! overflow)
			overflow = resource_class;
		
		resource_class = resource_class->parent;
	}
	return overflow;
}

void
jive_resource_class_count_sub(jive_resource_class_count * self, const struct jive_resource_class * resource_class)
{
	while(resource_class) {
		jive_resource_class_count_sub_single(self, resource_class);
		resource_class = resource_class->parent;
	}
}

const jive_resource_class *
jive_resource_class_count_change(jive_resource_class_count * self, const struct jive_resource_class * old_resource_class, const struct jive_resource_class * new_resource_class)
{
	jive_resource_class_count_sub(self, old_resource_class);
	return jive_resource_class_count_add(self, new_resource_class);
}

const jive_resource_class *
jive_resource_class_count_check_add(const jive_resource_class_count * self, const jive_resource_class * resource_class)
{
	while(resource_class) {
		size_t count = jive_resource_class_count_get(self, resource_class);
		if (count + 1 > resource_class->limit && resource_class->limit)
			return resource_class;
		resource_class = resource_class->parent;
	}
	return 0;
}

const jive_resource_class *
jive_resource_class_count_check_change(const jive_resource_class_count * self, const jive_resource_class * old_resource_class, const jive_resource_class * new_resource_class)
{
	if (!old_resource_class) return jive_resource_class_count_check_add(self, new_resource_class);
	if (!new_resource_class) return 0;
	
	const jive_resource_class * common_resource_class = jive_resource_class_union(old_resource_class, new_resource_class);
	
	while(new_resource_class != common_resource_class) {
		size_t count = jive_resource_class_count_get(self, new_resource_class);
		if (count + 1 > new_resource_class->limit && new_resource_class->limit)
			return new_resource_class;
		new_resource_class = new_resource_class->parent;
	}
	return 0;
}

void
jive_resource_class_count_copy(jive_resource_class_count * self, const jive_resource_class_count * src)
{
	jive_resource_class_count_clear(self);
	jive_resource_class_count_item * item;
	JIVE_LIST_ITERATE(src->items, item, item_list) {
		jive_resource_class_count_add_single(self, item->resource_class, item->count);
	}
}

bool
jive_resource_class_count_equals(const jive_resource_class_count * self, const jive_resource_class_count * other)
{
	if (self->nitems != other->nitems)
		return false;
	
	struct jive_resource_class_count_iterator i;
	for (i = jive_resource_class_count_begin(self); i.entry; jive_resource_class_count_iterator_next(&i)) {
		jive_resource_class_count_item * item = i.entry;
		if (item->count != jive_resource_class_count_get(other, item->resource_class))
			return false;
	}
	
	return true;
}

void
jive_resource_class_count_update_union(jive_resource_class_count * self, const jive_resource_class_count * other)
{
	jive_resource_class_count_item * other_item;
	JIVE_LIST_ITERATE(other->items, other_item, item_list) {
		jive_resource_class_count_item * item = jive_resource_class_count_lookup_item(self, other_item->resource_class);
		if (!item)
			jive_resource_class_count_add_single(self, other_item->resource_class, other_item->count);
		else
			item->count = item->count > other_item->count ? item->count : other_item->count;
	}
}

void
jive_resource_class_count_update_intersection(jive_resource_class_count * self, const jive_resource_class_count * other)
{
	jive_resource_class_count_item * item, * next_item;
	JIVE_LIST_ITERATE_SAFE(self->items, item, next_item, item_list) {
		size_t count = jive_resource_class_count_get(other, item->resource_class);
		if (!count) {
			size_t index = jive_ptr_hash(item->resource_class) & self->mask;
			JIVE_LIST_REMOVE(self->buckets[index], item, hash_chain);
			JIVE_LIST_REMOVE(self->items, item, item_list);
			delete item;
			self->nitems --;
		} else {
			item->count = item->count <  count ? item->count : count;
		}
	}
}

void
jive_resource_class_count_update_add(jive_resource_class_count * self, const jive_resource_class_count * other)
{
	jive_resource_class_count_item * item;
	JIVE_LIST_ITERATE(other->items, item, item_list) {
		jive_resource_class_count_add_single(self, item->resource_class, item->count);
	}
}

static inline size_t
max(size_t a, size_t b)
{
	return a > b ? a : b;
}

void
jive_rescls_prio_array_compute(jive_rescls_prio_array * self, const jive_resource_class_count * count)
{
	size_t n;
	for (n = 0; n < 8; n++)
		self->count[n] = 0;
	jive_resource_class_count_item * item;
	JIVE_LIST_ITERATE(count->items, item, item_list) {
		size_t index = (size_t) item->resource_class->priority;
		self->count[index] = max(self->count[index], item->count);
	}
}

int
jive_rescls_prio_array_compare(const jive_rescls_prio_array * self, const jive_rescls_prio_array * other)
{
	size_t n;
	for (n = 0; n < 8; n++) {
		if (self->count[n] < other->count[n])
			return -1;
		if (self->count[n] > other->count[n])
			return +1;
	}
	return 0;
}
