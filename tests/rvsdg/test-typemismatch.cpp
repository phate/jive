/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.hpp"
#include "testtypes.hpp"

#include <assert.h>

#include <jive/rvsdg.hpp>
#include <jive/view.hpp>

#include "testnodes.hpp"

static int test_main(void)
{
	using namespace jive;

	jive::graph graph;
	
	test::statetype type;
	test::valuetype value_type;

	auto n1 = test::simple_node_create(graph.root(), {}, {}, {type});

	bool error_handler_called = false;
	try {
		test::simple_node_create(graph.root(), {value_type}, {n1->output(0)}, {});
	} catch (jive::type_error & e) {
		error_handler_called = true;
	}
	
	assert(error_handler_called);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("rvsdg/test-typemismatch", test_main)
