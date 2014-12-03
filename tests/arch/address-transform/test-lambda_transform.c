/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/addresstype.h>
#include <jive/arch/memlayout-simple.h>
#include <jive/types/function/fctlambda.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>

#include "testnodes.h"

static int
test_main(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::addr::type addrtype;
	const jive::base::type * addrptr = &addrtype;
	const char * tmparray0[] = {"x"};

	jive_lambda * lambda = jive_lambda_begin(graph, 1, &addrptr, tmparray0);
	jive::output * fct = jive_lambda_end(lambda, 1, &addrptr, lambda->arguments);

	const jive::base::type * fcttype = &fct->type();
	jive_node * bottom = jive_test_node_create(graph->root_region,
		1, &fcttype, &fct,
		0, NULL);
	(void) bottom;

	jive_view(graph, stdout);

	jive_memlayout_mapper_simple mapper;
	jive_memlayout_mapper_simple_init(&mapper, 32);
	jive_node_address_transform(fct->node(), &mapper.base.base);
	jive_memlayout_mapper_simple_fini(&mapper);

	jive_view(graph, stdout);

	assert(bottom->producer(0)->operation() == jive::bitstring_to_address_operation(32, addrtype));

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/address-transform/test-lambda_transform", test_main);
