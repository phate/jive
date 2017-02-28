/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>

#include <jive/view.h>
#include <jive/vsdg.h>

#include "testnodes.h"

static int test_main(void)
{
	jive::graph graph;
	
	jive::region * region = graph.root();
	jive_test_state_type type;
	jive_test_value_type value_type;

	jive::node * n1 = jive_test_node_create(region, {}, {}, {&type});

	bool error_handler_called = false;
	try {
		jive_test_node_create(region, {&value_type}, {n1->output(0)}, {});
	} catch (jive::type_error e) {
		error_handler_called = true;
	}
	
	assert(error_handler_called);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-typemismatch", test_main);
