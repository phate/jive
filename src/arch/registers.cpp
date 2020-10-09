/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/registers.hpp>

namespace jive {

const resource_type register_resource(false, "register", &root_resource);

/* registers class */

registers::~registers()
{}

register_class::~register_class()
{}

}

const jive::resource_class jive_root_register_class(
	&jive::root_resource, "register", {},
	&jive_root_resource_class, jive::resource_class::priority::lowest,
	{}, nullptr);
