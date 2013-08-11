/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
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

	JIVE_DECLARE_BITSTRING_TYPE(bits32, 32);
	jive_lambda * lambda = jive_lambda_begin(graph,
		2, (const jive_type *[]){bits32, bits32}, (const char *[]){"arg1", "arg2"});

	jive_output * sum = jive_bitsum(lambda->narguments, lambda->arguments);

	jive_output * fct = jive_lambda_end(lambda, 1, &bits32, &sum);
	
	jive_view(graph, stderr);
	
	jive_function_type ftype;
	jive_function_type_init(&ftype, ctx, 2, (const jive_type *[]){bits32, bits32}, 1, &bits32);

	assert(jive_type_equals(&ftype.base.base, jive_output_get_type(fct)));
	
	jive_function_type_fini(&ftype);
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("function/test-build-lambda", test_main);
