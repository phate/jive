/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TEST_REGISTRY_H
#define JIVE_TEST_REGISTRY_H

void
jive_unit_test_register(const char * name, int (*fn)(void));

int
jive_unit_test_run(const char * name);

#define JIVE_UNIT_TEST_REGISTER(name, function) \
	static void __attribute__((constructor)) register_##function(void) \
	{ \
		jive_unit_test_register(name, function); \
	} \

#endif
