/*
 * Copyright 2010 2011 2012 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/resource.h>

#include <sstream>

#include <jive/internal/compiler.h>
#include <jive/util/list.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/type.h>

namespace jive {

resource_class::~resource_class()
{}

resource_name::~resource_name()
{}

}

const jive::resource_class *
jive_resource_class_union(const jive::resource_class * self, const jive::resource_class * other)
{
	for(;;) {
		if (self == other) return self;
		if (self->depth() > other->depth())
			self = self->parent();
		else
			other = other->parent();
	}
}

const jive::resource_class *
jive_resource_class_intersection(const jive::resource_class * self,
	const jive::resource_class * other)
{
	auto u = jive_resource_class_union(self, other);
	if (u == self) return other;
	else if (u == other) return self;
	else return 0;
}

const jive::resource_class *
jive_resource_class_relax(const jive::resource_class * self)
{
	/* hopefully this function is transitionary --
	currently everything that is needed is the
	class directly below the root */
	while (self->parent() && !jive_resource_class_is_abstract(self->parent()))
		self = self->parent();
	return self;
}

const jive_resource_class_class JIVE_ABSTRACT_RESOURCE = {
	parent : 0,
	name : "root",
	is_abstract : true
};

const jive::resource_class jive_root_resource_class(
	&JIVE_ABSTRACT_RESOURCE, "root", {}, nullptr,
	jive_resource_class_priority_lowest,
	{}, nullptr);
