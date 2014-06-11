/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/serialization/nodecls-registry.h>

#include <pthread.h>

#include <jive/util/buffer.h>
#include <jive/util/hash.h>
#include <jive/vsdg/node.h>

typedef struct jive_nodecls_tag_bucket jive_nodecls_tag_bucket;
struct jive_nodecls_tag_bucket {
	jive_serialization_nodecls * first;
	jive_serialization_nodecls * last;
};

typedef struct jive_nodecls_cls_bucket jive_nodecls_cls_bucket;
struct jive_nodecls_cls_bucket {
	jive_serialization_nodecls * first;
	jive_serialization_nodecls * last;
};

struct jive_serialization_nodecls_registry {
	jive_nodecls_tag_bucket * by_tag;
	jive_nodecls_cls_bucket * by_cls;
	size_t nbuckets;
	size_t nitems;
	size_t mask;
	bool initialized;
};

static jive_nodecls_tag_bucket jive_nodecls_empty_tag_bucket;
static jive_nodecls_cls_bucket jive_nodecls_empty_cls_bucket;
static jive_serialization_nodecls_registry nodecls_registry_singleton = {
	by_tag : &jive_nodecls_empty_tag_bucket,
	by_cls : &jive_nodecls_empty_cls_bucket,
	nbuckets : 0,
	nitems : 0,
	mask : 0,
	initialized : false
};
static pthread_mutex_t nodecls_registry_singleton_lock = PTHREAD_MUTEX_INITIALIZER;

static void
jive_nodecls_tag_chain_remove(jive_nodecls_tag_bucket * b, jive_serialization_nodecls * sercls)
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
jive_nodecls_tag_chain_insert(jive_nodecls_tag_bucket * b, jive_serialization_nodecls * sercls)
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
jive_nodecls_cls_chain_remove(jive_nodecls_cls_bucket * b, jive_serialization_nodecls * sercls)
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
jive_nodecls_cls_chain_insert(jive_nodecls_cls_bucket * b, jive_serialization_nodecls * sercls)
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
jive_nodecls_cls_hash(const jive_node_class * cls)
{
	return jive_ptr_hash(cls);
}

static bool
jive_serialization_nodecls_registry_rehash(
	jive_serialization_nodecls_registry * self)
{
	size_t mask = self->mask;
	size_t new_nbuckets = self->nbuckets * 2;
	if (!new_nbuckets)
		new_nbuckets = 1;
	size_t new_mask = new_nbuckets - 1;
	jive_nodecls_cls_bucket * new_by_cls = malloc(new_nbuckets * sizeof(*new_by_cls));
	if (!new_by_cls)
		return false;
	jive_nodecls_tag_bucket * new_by_tag = malloc(new_nbuckets * sizeof(*new_by_tag));
	if (!new_by_tag) {
		free(new_by_cls);
		return false;
	}
	
	size_t k;
	for (k = 0; k < new_nbuckets; ++k)
		new_by_cls[k].first = new_by_cls[k].last = 0;
	for (k = 0; k < new_nbuckets; ++k)
		new_by_tag[k].first = new_by_tag[k].last = 0;
	
	for (k = 0; k < self->nbuckets; k++) {
		jive_nodecls_tag_bucket * tb = &self->by_tag[k];
		while (tb->first) {
			jive_serialization_nodecls * tmp = tb->first;
			jive_nodecls_cls_bucket * cb = self->by_cls + (jive_nodecls_cls_hash(tmp->cls) & mask);
			jive_nodecls_tag_chain_remove(tb, tmp);
			jive_nodecls_cls_chain_remove(cb, tmp);
			
			jive_nodecls_tag_bucket * new_tb = new_by_tag + (jive_tag_hash(tmp->tag) & new_mask);
			jive_nodecls_cls_bucket * new_cb = new_by_cls + (jive_nodecls_cls_hash(tmp->cls) & new_mask);
			jive_nodecls_tag_chain_insert(new_tb, tmp);
			jive_nodecls_cls_chain_insert(new_cb, tmp);
		}
	}
	
	if (self->nbuckets) {
		free(self->by_tag);
		free(self->by_cls);
	}
	self->by_tag = new_by_tag;
	self->by_cls = new_by_cls;
	self->nbuckets = new_nbuckets;
	self->mask = new_mask;
	
	return true;
}

static bool
jive_serialization_nodecls_registry_insert(
	jive_serialization_nodecls_registry * self,
	jive_serialization_nodecls * sercls)
{
	if (self->nbuckets == self->nitems) {
		if (!jive_serialization_nodecls_registry_rehash(self))
			return false;
	}
	
	size_t mask = self->mask;
	
	JIVE_DEBUG_ASSERT(!jive_serialization_nodecls_lookup_by_tag(self, sercls->tag));
	JIVE_DEBUG_ASSERT(!jive_serialization_nodecls_lookup_by_cls(self, sercls->cls));
	
	jive_nodecls_tag_bucket * tb = self->by_tag + (jive_tag_hash(sercls->tag) & mask);
	jive_nodecls_cls_bucket * cb = self->by_cls + (jive_nodecls_cls_hash(sercls->cls) & mask);
	jive_nodecls_tag_chain_insert(tb, sercls);
	jive_nodecls_cls_chain_insert(cb, sercls);
	
	self->nitems ++;
	
	return true;
}

const jive_serialization_nodecls_registry *
jive_serialization_nodecls_registry_get(void)
{
	pthread_mutex_lock(&nodecls_registry_singleton_lock);
	jive_serialization_nodecls_registry * reg = &nodecls_registry_singleton;
	pthread_mutex_unlock(&nodecls_registry_singleton_lock);
	
	return reg;
}

const jive_serialization_nodecls *
jive_serialization_nodecls_lookup_by_cls(
	const jive_serialization_nodecls_registry * self,
	const jive_node_class * cls)
{
	jive_nodecls_cls_bucket *b = self->by_cls + (jive_nodecls_cls_hash(cls) & self->mask);
	const jive_serialization_nodecls * sercls = b->first;
	while (sercls) {
		if (sercls->cls == cls)
			return sercls;
		sercls = sercls->cls_chain.next;
	}
	
	return 0;
}

const jive_serialization_nodecls *
jive_serialization_nodecls_lookup_by_tag(
	const jive_serialization_nodecls_registry * self,
	const char * tag)
{
	jive_nodecls_tag_bucket * b = self->by_tag + (jive_tag_hash(tag) & self->mask);
	const jive_serialization_nodecls * sercls = b->first;
	while (sercls) {
		if (strcmp(sercls->tag, tag) == 0)
			return sercls;
		sercls = sercls->tag_chain.next;
	}
	
	return 0;
}

void
jive_serialization_nodecls_register(
	const jive_node_class * nodecls,
	const char tag[],
	jive_nodecls_serialize_function_t serialize,
	jive_nodecls_deserialize_function_t deserialize)
{
	jive_serialization_nodecls * sercls = malloc(sizeof(*sercls));
	sercls->tag = strdup(tag);
	sercls->cls = nodecls;
	sercls->serialize = serialize;
	sercls->deserialize = deserialize;
	pthread_mutex_lock(&nodecls_registry_singleton_lock);
	jive_serialization_nodecls_registry_insert(&nodecls_registry_singleton, sercls);
	pthread_mutex_unlock(&nodecls_registry_singleton_lock);
}

void
jive_serialization_nodecls_serialize_default(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	const jive_node_attrs * attrs,
	struct jive_token_ostream * os)
{
}

bool
jive_serialization_nodecls_deserialize_default(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	jive_region * region,
	size_t noperands, jive::output * const operands[],
	struct jive_token_istream * is,
	jive_node ** node)
{
	*node = self->cls->create(region, NULL, noperands, operands);
	return *node != 0;
}
