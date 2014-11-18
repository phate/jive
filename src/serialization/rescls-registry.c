/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/serialization/rescls-registry.h>

#include <pthread.h>

#include <jive/arch/registers.h>
#include <jive/serialization/token-stream.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/resource.h>

static inline size_t
jive_ptr_hash(const void * ptr)
{
	/* hm, ideally I would like to "rotate" instead of "shifting"... */
	size_t hash = (size_t) ptr;
	hash ^= (hash >> 20) ^ (hash >> 12);
	return hash ^ (hash >> 7) ^ (hash >> 4);
}

typedef struct jive_rescls_tag_bucket jive_rescls_tag_bucket;
struct jive_rescls_tag_bucket {
	jive_serialization_rescls * first;
	jive_serialization_rescls * last;
};

typedef struct jive_rescls_cls_bucket jive_rescls_cls_bucket;
struct jive_rescls_cls_bucket {
	jive_serialization_rescls * first;
	jive_serialization_rescls * last;
};

struct jive_serialization_rescls_registry {
	jive_rescls_tag_bucket * by_tag;
	jive_rescls_cls_bucket * by_cls;
	size_t nbuckets;
	size_t nitems;
	size_t mask;
	bool initialized;
};

static jive_rescls_tag_bucket jive_rescls_empty_tag_bucket;
static jive_rescls_cls_bucket jive_rescls_empty_cls_bucket;
static jive_serialization_rescls_registry rescls_registry_singleton = {
	by_tag : &jive_rescls_empty_tag_bucket,
	by_cls : &jive_rescls_empty_cls_bucket,
	nbuckets : 0,
	nitems : 0,
	mask : 0,
	initialized : false
};
static pthread_mutex_t rescls_registry_singleton_lock = PTHREAD_MUTEX_INITIALIZER;

static void
jive_rescls_tag_chain_remove(jive_rescls_tag_bucket * b, jive_serialization_rescls * sercls)
{
	if (sercls->tag_chain.prev)
		sercls->tag_chain.prev->tag_chain.next = sercls->tag_chain.next;
	else
		b->first = sercls->tag_chain.next;
	if (sercls->tag_chain.next)
		sercls->tag_chain.next->tag_chain.prev = sercls->tag_chain.prev;
	else
		b->last = sercls->tag_chain.prev;
}

static void
jive_rescls_tag_chain_insert(jive_rescls_tag_bucket * b, jive_serialization_rescls * sercls)
{
	sercls->tag_chain.prev = b->last;
	sercls->tag_chain.next = 0;
	if (b->last)
		b->last->tag_chain.next = sercls;
	else
		b->first = sercls;
	b->last = sercls;
}

static size_t
jive_tag_hash(const char * tag)
{
	size_t tmp = 0;
	while (*tag) {
		tmp = tmp * 5 + * (unsigned char *) tag;
		++tag;
	}
	
	return tmp;
}

static void
jive_rescls_cls_chain_remove(jive_rescls_cls_bucket * b, jive_serialization_rescls * sercls)
{
	if (sercls->cls_chain.prev)
		sercls->cls_chain.prev->cls_chain.next = sercls->cls_chain.next;
	else
		b->first = sercls->cls_chain.next;
	if (sercls->cls_chain.next)
		sercls->cls_chain.next->cls_chain.prev = sercls->cls_chain.prev;
	else
		b->last = sercls->cls_chain.prev;
}

static void
jive_rescls_cls_chain_insert(jive_rescls_cls_bucket * b, jive_serialization_rescls * sercls)
{
	sercls->cls_chain.prev = b->last;
	sercls->cls_chain.next = 0;
	if (b->last)
		b->last->cls_chain.next = sercls;
	else
		b->first = sercls;
	b->last = sercls;
}

static size_t
jive_rescls_cls_hash(const void * cls)
{
	return jive_ptr_hash(cls);
}

static bool
jive_serialization_rescls_registry_rehash(
	jive_serialization_rescls_registry * self)
{
	size_t mask = self->mask;
	size_t new_nbuckets = self->nbuckets * 2;
	if (!new_nbuckets)
		new_nbuckets = 1;
	size_t new_mask = new_nbuckets - 1;
	jive_rescls_cls_bucket * new_by_cls = new jive_rescls_cls_bucket[new_nbuckets];
	if (!new_by_cls)
		return false;
	jive_rescls_tag_bucket * new_by_tag = new jive_rescls_tag_bucket[new_nbuckets];
	if (!new_by_tag) {
		delete[] new_by_cls;
		return false;
	}
	
	size_t k;
	for (k = 0; k < new_nbuckets; ++k)
		new_by_cls[k].first = new_by_cls[k].last = 0;
	for (k = 0; k < new_nbuckets; ++k)
		new_by_tag[k].first = new_by_tag[k].last = 0;
	
	for (k = 0; k < self->nbuckets; k++) {
		jive_rescls_tag_bucket * tb = &self->by_tag[k];
		while (tb->first) {
			jive_serialization_rescls * tmp = tb->first;
			jive_rescls_cls_bucket * cb = self->by_cls + (jive_rescls_cls_hash(tmp->cls) & mask);
			jive_rescls_tag_chain_remove(tb, tmp);
			jive_rescls_cls_chain_remove(cb, tmp);
			
			jive_rescls_tag_bucket * new_tb = new_by_tag + (jive_tag_hash(tmp->tag) & new_mask);
			jive_rescls_cls_bucket * new_cb = new_by_cls + (jive_rescls_cls_hash(tmp->cls) & new_mask);
			jive_rescls_tag_chain_insert(new_tb, tmp);
			jive_rescls_cls_chain_insert(new_cb, tmp);
		}
	}
	
	if (self->nbuckets) {
		delete[] self->by_tag;
		delete[] self->by_cls;
	}
	self->by_tag = new_by_tag;
	self->by_cls = new_by_cls;
	self->nbuckets = new_nbuckets;
	self->mask = new_mask;
	
	return true;
}

static bool
jive_serialization_rescls_exists(
	const jive_serialization_rescls_registry * self,
	const void * cls,
	bool is_meta)
{
	jive_rescls_cls_bucket *b = self->by_cls + (jive_rescls_cls_hash(cls) & self->mask);
	const jive_serialization_rescls * sercls = b->first;
	while (sercls) {
		if ((sercls->cls == cls) && (sercls->is_meta_class == is_meta))
			return true;
		sercls = sercls->cls_chain.next;
	}
	return false;
}

static bool
jive_serialization_rescls_registry_insert(
	jive_serialization_rescls_registry * self,
	jive_serialization_rescls * sercls)
{
	if (self->nbuckets == self->nitems) {
		if (!jive_serialization_rescls_registry_rehash(self))
			return false;
	}
	
	size_t mask = self->mask;
	
	JIVE_DEBUG_ASSERT(!jive_serialization_rescls_lookup_by_tag(self, sercls->tag));
	JIVE_DEBUG_ASSERT(!jive_serialization_rescls_exists(self, sercls->cls, sercls->is_meta_class));
	
	jive_rescls_tag_bucket * tb = self->by_tag + (jive_tag_hash(sercls->tag) & mask);
	jive_rescls_cls_bucket * cb = self->by_cls + (jive_rescls_cls_hash(sercls->cls) & mask);
	jive_rescls_tag_chain_insert(tb, sercls);
	jive_rescls_cls_chain_insert(cb, sercls);
	
	self->nitems ++;
	
	return true;
}

const jive_serialization_rescls_registry *
jive_serialization_rescls_registry_get(void)
{
	pthread_mutex_lock(&rescls_registry_singleton_lock);
	jive_serialization_rescls_registry * reg = &rescls_registry_singleton;
	pthread_mutex_unlock(&rescls_registry_singleton_lock);
	
	return reg;
}

const jive_serialization_rescls *
jive_serialization_rescls_lookup_by_cls(
	const jive_serialization_rescls_registry * self,
	const jive_resource_class * cls)
{
	jive_rescls_cls_bucket *b = self->by_cls + (jive_rescls_cls_hash(cls) & self->mask);
	const jive_serialization_rescls * sercls = b->first;
	while (sercls) {
		if ((sercls->cls == cls) && !sercls->is_meta_class)
			return sercls;
		sercls = sercls->cls_chain.next;
	}
	
	const jive_resource_class_class * meta = cls->class_;
	b = self->by_cls + (jive_rescls_cls_hash(meta) & self->mask);
	sercls = b->first;
	while (sercls) {
		if ((sercls->cls == meta) && sercls->is_meta_class)
			return sercls;
		sercls = sercls->cls_chain.next;
	}
	
	return 0;
}

const jive_serialization_rescls *
jive_serialization_rescls_lookup_by_tag(
	const jive_serialization_rescls_registry * self,
	const char * tag)
{
	jive_rescls_tag_bucket * b = self->by_tag + (jive_tag_hash(tag) & self->mask);
	const jive_serialization_rescls * sercls = b->first;
	while (sercls) {
		if (strcmp(sercls->tag, tag) == 0)
			return sercls;
		sercls = sercls->tag_chain.next;
	}
	
	return 0;
}

void
jive_serialization_rescls_register(
	const void * entity,
	const char tag[],
	bool is_meta_class,
	jive_rescls_serialize_function_t serialize,
	jive_rescls_deserialize_function_t deserialize)
{
	jive_serialization_rescls * sercls = new jive_serialization_rescls;
	sercls->tag = strdup(tag);
	sercls->cls = entity;
	sercls->is_meta_class = is_meta_class;
	sercls->serialize = serialize;
	sercls->deserialize = deserialize;
	pthread_mutex_lock(&rescls_registry_singleton_lock);
	jive_serialization_rescls_registry_insert(&rescls_registry_singleton, sercls);
	pthread_mutex_unlock(&rescls_registry_singleton_lock);
}

void
jive_serialization_regclsset_register(
	const struct jive_register_class * const * regclsset,
	size_t count,
	const char prefix[])
{
	size_t prefix_len = strlen(prefix);
	pthread_mutex_lock(&rescls_registry_singleton_lock);
	size_t n;
	for (n = 0; n < count; ++n) {
		const jive_register_class * regcls = regclsset[n];
		if (!regcls->base.name)
			continue;
		size_t name_len = strlen(regcls->base.name);
		char * tag = malloc(prefix_len + name_len + 1);
		memcpy(tag, prefix, prefix_len);
		memcpy(tag + prefix_len, regcls->base.name, name_len);
		tag[prefix_len + name_len] = 0;
		jive_serialization_rescls * sercls = new jive_serialization_rescls;
		sercls->tag = tag;
		sercls->cls = regcls;
		sercls->is_meta_class = false;
		sercls->serialize = jive_serialization_rescls_serialize_default;
		sercls->deserialize = jive_serialization_rescls_deserialize_default;
		jive_serialization_rescls_registry_insert(&rescls_registry_singleton, sercls);
	}
	pthread_mutex_unlock(&rescls_registry_singleton_lock);
}

void
jive_serialization_rescls_serialize_default(
	const jive_serialization_rescls * self,
	struct jive_serialization_driver * driver,
	const struct jive_resource_class * rescls,
	struct jive_token_ostream * os)
{
}

bool
jive_serialization_rescls_deserialize_default(
	const jive_serialization_rescls * self,
	struct jive_serialization_driver * driver,
	struct jive_token_istream * is,
	const struct jive_resource_class ** rescls)
{
	*rescls = (const jive_resource_class *) self->cls;
	return true;
}
