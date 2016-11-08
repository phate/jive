/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
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

	jive_lambda * lambda = jive_lambda_begin(graph->root(), 1, &addrptr, tmparray0);
	jive::output * fct = jive_lambda_end(lambda, 1, &addrptr, lambda->arguments);

	jive_graph_export(graph, fct, "fct");

	jive_view(graph, stdout);

	jive::memlayout_mapper_simple mapper(4);
	jive_graph_address_transform(graph, &mapper);

	jive_view(graph, stdout);

	jive::bits::type bits32(32);
	const jive::base::type * tmp = &bits32;
	jive::fct::type fcttype(1, &tmp, 1, &tmp);
	assert(graph->root()->bottom()->input(0)->type() == fcttype);

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/address-transform/test-lambda_transform", test_main);
