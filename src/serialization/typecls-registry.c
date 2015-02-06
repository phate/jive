/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/serialization/typecls-registry.h>

#include <pthread.h>

#include <memory>

#include <jive/util/buffer.h>
#include <jive/util/cstr-map.h>
#include <jive/util/typeinfo-map.h>
#include <jive/vsdg/basetype.h>

struct jive_serialization_typecls_registry {
	~jive_serialization_typecls_registry();

	jive::detail::cstr_map<jive_serialization_typecls *> by_tag_;
	jive::detail::typeinfo_map<jive_serialization_typecls *> by_cls_;
};

jive_serialization_typecls_registry::~jive_serialization_typecls_registry()
{
	for (auto element : by_cls_) {
		jive_serialization_typecls * sercls = element.second;
		free((char*)(sercls->tag));
		delete sercls;
	}
}

static std::unique_ptr<jive_serialization_typecls_registry> typecls_registry_singleton;
static pthread_mutex_t typecls_registry_singleton_lock = PTHREAD_MUTEX_INITIALIZER;

static bool
jive_serialization_typecls_registry_insert(
	jive_serialization_typecls_registry * self,
	jive_serialization_typecls * sercls)
{
	self->by_tag_[sercls->tag] = sercls;
	self->by_cls_[sercls->cls] = sercls;
	
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
	auto i = self->by_tag_.find(tag);
	if (i != self->by_tag_.end()) {
		return i->second;
	} else {
		return nullptr;
	}
}

void
jive_serialization_typecls_register(
	const std::type_info & typecls,
	const char tag[],
	jive_typecls_serialize_function_t serialize,
	jive_typecls_deserialize_function_t deserialize)
{
	jive_serialization_typecls * sercls = new jive_serialization_typecls;
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
