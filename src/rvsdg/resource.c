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

const jive::resource_class jive_root_resource_class(
	&jive::root_resource, "root", {}, nullptr,
	jive_resource_class_priority_lowest,
	{}, nullptr);
