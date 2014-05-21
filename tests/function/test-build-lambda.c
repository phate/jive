/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/types/function/fctlambda.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);

	jive_bitstring_type bits32(32);
	const jive_type * tmparray0[] = {&bits32, &bits32};
	const char * tmparray1[] = {"arg1", "arg2"};
	jive_lambda * lambda = jive_lambda_begin(graph,
		2, tmparray0, tmparray1);

	jive_output * sum = jive_bitsum(lambda->narguments, lambda->arguments);

	jive_output * fct = jive_lambda_end(lambda, 1, tmparray0, &sum);
	
	jive_view(graph, stderr);
	
	const jive_type * tmparray2[] = {&bits32, &bits32};
	jive_function_type ftype(2, tmparray2, 1, tmparray2);

	assert(ftype == fct->type());
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("function/test-build-lambda", test_main);
