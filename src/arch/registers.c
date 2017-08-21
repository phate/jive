/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/registers.h>

#include <jive/vsdg/basetype.h>

jive_register_name::~jive_register_name()
{}

jive_register_class::~jive_register_class()
{}

const struct jive::base::type *
jive_register_class_get_type(const jive_register_class * self)
{
	return jive_resource_class_get_type(self);
}

static const jive_resource_class_demotion no_demotion[] = {{NULL, NULL}};

const jive_resource_class_class JIVE_REGISTER_RESOURCE = {
	parent : &JIVE_ABSTRACT_RESOURCE,
	name : "register",
	is_abstract : false
};

const jive_resource_class jive_root_register_class(
	&JIVE_ABSTRACT_RESOURCE, "register", {},
	&jive_root_resource_class, jive_resource_class_priority_lowest,
	no_demotion, nullptr);
