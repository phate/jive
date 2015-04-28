/*
 * Copyright 2010 2011 2012 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/resource.h>

#include <sstream>

#include <jive/internal/compiler.h>
#include <jive/util/list.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/graph.h>

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
jive_resource_class_intersection(const jive_resource_class * self,
	const jive_resource_class * other)
{
	const jive_resource_class * u = jive_resource_class_union(self, other);
	if (u == self) return other;
	else if (u == other) return self;
	else return 0;
}

jive::gate *
jive_resource_class_create_gate(const jive_resource_class * self, jive_graph * graph,
	const char * name)
{
	const jive::base::type * type = jive_resource_class_get_type(self);
	jive::gate * gate = jive_graph_create_gate(graph, name, *type);
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

static inline size_t
max(size_t a, size_t b)
{
	return a > b ? a : b;
}

void
jive_rescls_prio_array_compute(jive_rescls_prio_array * self,
	const jive_resource_class_count * count)
{
	size_t n;
	for (n = 0; n < 8; n++) {
		self->count[n] = 0;
	}
	for (const auto & item : count->counts()) {
		size_t index = static_cast<size_t>(item.first->priority);
		self->count[index] = std::max(
			self->count[index], static_cast<uint16_t>(item.second));
	}
}

int
jive_rescls_prio_array_compare(const jive_rescls_prio_array * self,
	const jive_rescls_prio_array * other)
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

/* generate human-readable representation */
std::string
jive_resource_class_count::debug_string() const
{
	bool first = true;
	std::ostringstream os;
	for (const auto& item : counts_) {
		if (!first) {
			os << ", ";
		}
		first = false;
		os << item.first->name << ":" << item.second;
	}
	return os.str();
}
