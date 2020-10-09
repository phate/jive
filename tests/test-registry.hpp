/*
 * Copyright 2010 2011 2012 2015 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TEST_REGISTRY_HPP
#define JIVE_TEST_REGISTRY_HPP

#include <string>
#include <vector>

void
jive_unit_test_register(const char * name, int (*fn)(void));

int
jive_unit_test_run(const char * name);

#define JIVE_UNIT_TEST_REGISTER(name, function) \
	static void __attribute__((constructor)) register_##function(void) \
	{ \
		jive_unit_test_register(name, function); \
	} \

std::vector<std::string>
list_unit_tests();

#endif
