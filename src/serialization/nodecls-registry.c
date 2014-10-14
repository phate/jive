/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/serialization/nodecls-registry.h>

#include <mutex>

#include <pthread.h> // FIXME: remove

#include <jive/util/buffer.h>
#include <jive/util/hash.h>
#include <jive/vsdg/node.h>

namespace jive {
namespace serialization {

opcls_handler::~opcls_handler() noexcept
{
	registry_.cls_hash_.erase(this);
	registry_.tag_hash_.erase(this);
}

namespace {
opcls_registry * registry_singleton = nullptr;
std::mutex registry_singleton_lock;
}

opcls_registry &
opcls_registry::mutable_instance()
{
	std::lock_guard<std::mutex> guard(registry_singleton_lock);
	if (!registry_singleton) {
		registry_singleton = new opcls_registry();
	}
	return *registry_singleton;
}

const opcls_registry &
opcls_registry::instance()
{
	return mutable_instance();
}

}
}
