/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/serialization/typecls-registry.h>

#include <pthread.h>

#include <jive/util/buffer.h>
#include <jive/util/hash.h>
#include <jive/util/typeinfo-map.h>
#include <jive/vsdg/basetype.h>

typedef struct jive_typecls_tag_bucket jive_typecls_tag_bucket;
struct jive_typecls_tag_bucket {
	jive_serialization_typecls * first;
	jive_serialization_typecls * last;
};

static jive_typecls_tag_bucket jive_typecls_empty_tag_bucket;
struct jive_serialization_typecls_registry {
	inline jive_serialization_typecls_registry()
		: by_tag(&jive_typecls_empty_tag_bucket)
		, nbuckets(0)
		, nitems(0)
		, mask(0)
		, initialized(false) {
	}
	jive_typecls_tag_bucket * by_tag;
	size_t nbuckets;
	size_t nitems;
	size_t mask;
	bool initialized;
	jive::detail::typeinfo_map<const jive_serialization_typecls *> by_cls_;
};

static std::unique_ptr<jive_serialization_typecls_registry> typecls_registry_singleton;
static pthread_mutex_t typecls_registry_singleton_lock = PTHREAD_MUTEX_INITIALIZER;

static void
jive_typecls_tag_chain_remove(jive_typecls_tag_bucket * b, jive_serialization_typecls * sercls)
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
jive_typecls_tag_chain_insert(jive_typecls_tag_bucket * b, jive_serialization_typecls * sercls)
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

static bool
jive_serialization_typecls_registry_rehash(
	jive_serialization_typecls_registry * self)
{
	size_t mask = self->mask;
	size_t new_nbuckets = self->nbuckets * 2;
	if (!new_nbuckets)
		new_nbuckets = 1;
	size_t new_mask = new_nbuckets - 1;
	jive_typecls_tag_bucket * new_by_tag = malloc(new_nbuckets * sizeof(*new_by_tag));
	if (!new_by_tag) {
		return false;
	}
	
	size_t k;
	for (k = 0; k < new_nbuckets; ++k)
		new_by_tag[k].first = new_by_tag[k].last = 0;
	
	for (k = 0; k < self->nbuckets; k++) {
		jive_typecls_tag_bucket * tb = &self->by_tag[k];
		while (tb->first) {
			jive_serialization_typecls * tmp = tb->first;
			jive_typecls_tag_chain_remove(tb, tmp);
			
			jive_typecls_tag_bucket * new_tb = new_by_tag + (jive_tag_hash(tmp->tag) & new_mask);
			jive_typecls_tag_chain_insert(new_tb, tmp);
		}
	}
	
	if (self->nbuckets) {
		free(self->by_tag);
	}
	self->by_tag = new_by_tag;
	self->nbuckets = new_nbuckets;
	self->mask = new_mask;
	
	return true;
}

static bool
jive_serialization_typecls_registry_insert(
	jive_serialization_typecls_registry * self,
	jive_serialization_typecls * sercls)
{
	if (self->nbuckets == self->nitems) {
		if (!jive_serialization_typecls_registry_rehash(self))
			return false;
	}
	
	size_t mask = self->mask;
	
	JIVE_DEBUG_ASSERT(!jive_serialization_typecls_lookup_by_tag(self, sercls->tag));
	
	jive_typecls_tag_bucket * tb = self->by_tag + (jive_tag_hash(sercls->tag) & mask);
	jive_typecls_tag_chain_insert(tb, sercls);
	
	self->by_cls_[sercls->cls] = sercls;
	
	self->nitems ++;
	
	return true;
}

const jive_serialization_typecls_registry *
jive_serialization_typecls_registry_get(void)
{
	pthread_mutex_lock(&typecls_registry_singleton_lock);
	if (!typecls_registry_singleton) {
		typecls_registry_singleton.reset(new jive_serialization_typecls_registry());
	}
	jive_serialization_typecls_registry * reg = typecls_registry_singleton.get();
	pthread_mutex_unlock(&typecls_registry_singleton_lock);
	
	return reg;
}

const jive_serialization_typecls *
jive_serialization_typecls_lookup_by_cls(
	const jive_serialization_typecls_registry * self,
	const std::type_info & cls)
{
	auto i = self->by_cls_.find(&cls);
	if (i != self->by_cls_.end()) {
		return i->second;
	} else {
		return nullptr;
	}
}

const jive_serialization_typecls *
jive_serialization_typecls_lookup_by_tag(
	const jive_serialization_typecls_registry * self,
	const char * tag)
{
	jive_typecls_tag_bucket * b = self->by_tag + (jive_tag_hash(tag) & self->mask);
	const jive_serialization_typecls * sercls = b->first;
	while (sercls) {
		if (strcmp(sercls->tag, tag) == 0)
			return sercls;
		sercls = sercls->tag_chain.next;
	}
	
	return 0;
}

void
jive_serialization_typecls_register(
	const std::type_info & typecls,
	const char tag[],
	jive_typecls_serialize_function_t serialize,
	jive_typecls_deserialize_function_t deserialize)
{
	jive_serialization_typecls * sercls = malloc(sizeof(*sercls));
	sercls->tag = strdup(tag);
	sercls->cls = &typecls;
	sercls->serialize = serialize;
	sercls->deserialize = deserialize;
	pthread_mutex_lock(&typecls_registry_singleton_lock);
	if (!typecls_registry_singleton) {
		typecls_registry_singleton.reset(new jive_serialization_typecls_registry());
	}
	jive_serialization_typecls_registry_insert(typecls_registry_singleton.get(), sercls);
	pthread_mutex_unlock(&typecls_registry_singleton_lock);
}
