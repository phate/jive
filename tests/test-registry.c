/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <stdlib.h>
#include <string.h>

#include <jive/util/list.h>

typedef struct jive_unit_test jive_unit_test;

struct jive_unit_test {
	const char * name;
	int (*function)(void);
	struct {
		jive_unit_test * prev;
		jive_unit_test * next;
	} chain;
};

static struct {
	jive_unit_test * first;
	jive_unit_test * last;
} unit_tests = {0, 0};

void
jive_unit_test_register(const char * name, int (*fn)(void))
{
	jive_unit_test * test = malloc(sizeof(*test));
	test->name = name;
	test->function = fn;
	JIVE_LIST_PUSH_BACK(unit_tests, test, chain);
}

static const jive_unit_test *
jive_unit_test_lookup(const char * name)
{
	jive_unit_test * test;
	JIVE_LIST_ITERATE(unit_tests, test, chain) {
		if (strcmp(test->name, name) == 0)
			return test;
	}
	return 0;
}

int
jive_unit_test_run(const char * name)
{
	const jive_unit_test * test = jive_unit_test_lookup(name);
	return test->function();
}
