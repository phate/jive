/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/serialization/instrcls-registry.h>

#include <pthread.h>

#include <jive/arch/instruction.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/node.h>

static inline size_t
jive_ptr_hash(const void * ptr)
{
	/* FIXME: hm, ideally I would like to "rotate" instead of "shifting"... */
	size_t hash = (size_t) ptr;
	hash ^= (hash >> 20) ^ (hash >> 12);
	return hash ^ (hash >> 7) ^ (hash >> 4);
}

typedef struct jive_instrcls_tag_bucket jive_instrcls_tag_bucket;
struct jive_instrcls_tag_bucket {
	jive_serialization_instrcls * first;
	jive_serialization_instrcls * last;
};

typedef struct jive_instrcls_cls_bucket jive_instrcls_cls_bucket;
struct jive_instrcls_cls_bucket {
	jive_serialization_instrcls * first;
	jive_serialization_instrcls * last;
};

struct jive_serialization_instrcls_registry {
	jive_instrcls_tag_bucket * by_tag;
	jive_instrcls_cls_bucket * by_cls;
	size_t nbuckets;
	size_t nitems;
	size_t mask;
	bool initialized;
};

static jive_instrcls_tag_bucket jive_instrcls_empty_tag_bucket;
static jive_instrcls_cls_bucket jive_instrcls_empty_cls_bucket;
static jive_serialization_instrcls_registry instrcls_registry_singleton = {
	by_tag : &jive_instrcls_empty_tag_bucket,
	by_cls : &jive_instrcls_empty_cls_bucket,
	nbuckets : 0,
	nitems : 0,
	mask : 0,
	initialized : false
};
static pthread_mutex_t instrcls_registry_singleton_lock = PTHREAD_MUTEX_INITIALIZER;

static void
jive_instrcls_tag_chain_remove(jive_instrcls_tag_bucket * b, jive_serialization_instrcls * sercls)
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
jive_instrcls_tag_chain_insert(jive_instrcls_tag_bucket * b, jive_serialization_instrcls * sercls)
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
jive_instrcls_cls_chain_remove(jive_instrcls_cls_bucket * b, jive_serialization_instrcls * sercls)
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
jive_instrcls_cls_chain_insert(jive_instrcls_cls_bucket * b, jive_serialization_instrcls * sercls)
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
jive_instrcls_cls_hash(const struct jive_instruction_class * cls)
{
	return jive_ptr_hash(cls);
}

static bool
jive_serialization_instrcls_registry_rehash(
	jive_serialization_instrcls_registry * self)
{
	size_t mask = self->mask;
	size_t new_nbuckets = self->nbuckets * 2;
	if (!new_nbuckets)
		new_nbuckets = 1;
	size_t new_mask = new_nbuckets - 1;
	jive_instrcls_cls_bucket * new_by_cls = new jive_instrcls_cls_bucket[new_nbuckets];
	if (!new_by_cls)
		return false;
	jive_instrcls_tag_bucket * new_by_tag = new jive_instrcls_tag_bucket[new_nbuckets];
	if (!new_by_tag) {
		delete[] new_by_tag;
		return false;
	}
	
	size_t k;
	for (k = 0; k < new_nbuckets; ++k)
		new_by_cls[k].first = new_by_cls[k].last = 0;
	for (k = 0; k < new_nbuckets; ++k)
		new_by_tag[k].first = new_by_tag[k].last = 0;
	
	for (k = 0; k < self->nbuckets; k++) {
		jive_instrcls_tag_bucket * tb = &self->by_tag[k];
		while (tb->first) {
			jive_serialization_instrcls * tmp = tb->first;
			jive_instrcls_cls_bucket * cb = self->by_cls + (jive_instrcls_cls_hash(tmp->cls) & mask);
			jive_instrcls_tag_chain_remove(tb, tmp);
			jive_instrcls_cls_chain_remove(cb, tmp);
			
			jive_instrcls_tag_bucket * new_tb = new_by_tag + (jive_tag_hash(tmp->tag) & new_mask);
			jive_instrcls_cls_bucket * new_cb = new_by_cls + (jive_instrcls_cls_hash(tmp->cls) & new_mask);
			jive_instrcls_tag_chain_insert(new_tb, tmp);
			jive_instrcls_cls_chain_insert(new_cb, tmp);
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
jive_serialization_instrcls_registry_insert(
	jive_serialization_instrcls_registry * self,
	jive_serialization_instrcls * sercls)
{
	if (self->nbuckets == self->nitems) {
		if (!jive_serialization_instrcls_registry_rehash(self))
			return false;
	}
	
	size_t mask = self->mask;
	
	JIVE_DEBUG_ASSERT(!jive_serialization_instrcls_lookup_by_tag(self, sercls->tag));
	JIVE_DEBUG_ASSERT(!jive_serialization_instrcls_lookup_by_cls(self, sercls->cls));
	
	jive_instrcls_tag_bucket * tb = self->by_tag + (jive_tag_hash(sercls->tag) & mask);
	jive_instrcls_cls_bucket * cb = self->by_cls + (jive_instrcls_cls_hash(sercls->cls) & mask);
	jive_instrcls_tag_chain_insert(tb, sercls);
	jive_instrcls_cls_chain_insert(cb, sercls);
	
	self->nitems ++;
	
	return true;
}

const jive_serialization_instrcls_registry *
jive_serialization_instrcls_registry_get(void)
{
	pthread_mutex_lock(&instrcls_registry_singleton_lock);
	jive_serialization_instrcls_registry * reg = &instrcls_registry_singleton;
	pthread_mutex_unlock(&instrcls_registry_singleton_lock);
	
	return reg;
}

const jive_serialization_instrcls *
jive_serialization_instrcls_lookup_by_cls(
	const jive_serialization_instrcls_registry * self,
	const struct jive_instruction_class * cls)
{
	jive_instrcls_cls_bucket *b = self->by_cls + (jive_instrcls_cls_hash(cls) & self->mask);
	const jive_serialization_instrcls * sercls = b->first;
	while (sercls) {
		if (sercls->cls == cls)
			return sercls;
		sercls = sercls->cls_chain.next;
	}
	
	return 0;
}

const jive_serialization_instrcls *
jive_serialization_instrcls_lookup_by_tag(
	const jive_serialization_instrcls_registry * self,
	const char * tag)
{
	jive_instrcls_tag_bucket * b = self->by_tag + (jive_tag_hash(tag) & self->mask);
	const jive_serialization_instrcls * sercls = b->first;
	while (sercls) {
		if (strcmp(sercls->tag, tag) == 0)
			return sercls;
		sercls = sercls->tag_chain.next;
	}
	
	return 0;
}

void
jive_serialization_instrcls_register(
	const struct jive_instruction_class * instrcls,
	const char tag[])
{
	jive_serialization_instrcls * sercls = new jive_serialization_instrcls;
	sercls->tag = strdup(tag);
	sercls->cls = instrcls;
	pthread_mutex_lock(&instrcls_registry_singleton_lock);
	jive_serialization_instrcls_registry_insert(&instrcls_registry_singleton, sercls);
	pthread_mutex_unlock(&instrcls_registry_singleton_lock);
}

void
jive_serialization_instrset_register(
	const struct jive_instruction_class * const * instrclss,
	size_t ninstrclss,
	const char prefix[])
{
	pthread_mutex_lock(&instrcls_registry_singleton_lock);
	size_t prefix_len = strlen(prefix);
	size_t n;
	for (n = 0; n < ninstrclss; ++n) {
		const struct jive_instruction_class * icls = instrclss[n];
		if (!icls->name)
			continue;
		jive_serialization_instrcls * sercls = new jive_serialization_instrcls;
		size_t name_len = strlen(icls->name);
		char * tag = new char[prefix_len + name_len + 1];
		memcpy(tag, prefix, prefix_len);
		memcpy(tag + prefix_len, icls->name, name_len);
		tag[prefix_len + name_len] = 0;
		sercls->tag = tag;
		sercls->cls = icls;
		jive_serialization_instrcls_registry_insert(&instrcls_registry_singleton, sercls);
	}
	pthread_mutex_unlock(&instrcls_registry_singleton_lock);
}
