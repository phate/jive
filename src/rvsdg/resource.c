/*
 * Copyright 2010 2011 2012 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <sstream>

#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/resource.h>
#include <jive/rvsdg/type.h>

namespace jive {

resource_class::~resource_class()
{}

/* resource */

resource::~resource()
{}

/* resource_type */

resource_type::~resource_type()
{}

const resource_type root_resource(true, "root", nullptr);

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

const jive::resource_class jive_root_resource_class(
	&jive::root_resource, "root", {}, nullptr,
	jive_resource_class_priority_lowest,
	{}, nullptr);
