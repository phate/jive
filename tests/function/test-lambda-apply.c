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
#include <jive/types/function/fctapply.h>
#include <jive/types/function/fctlambda.h>
#include <jive/vsdg/node-private.h>

#include "testnodes.h"

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

	const jive_type * tmparray11[] = {&bits32};
	jive_output * lambda_expr = jive_lambda_end(lambda, 1, tmparray11, &sum);
	
	jive_output * c0 = jive_bitconstant(graph, 32, "01010100000000000000000000000000");
	jive_output * c1 = jive_bitconstant(graph, 32, "10010010000000000000000000000000");
	jive_output * tmparray2[] = {c0, c1};
	
	jive_node * apply_node = jive_apply_node_create(graph->root_region,
		lambda_expr, 2, tmparray2);
	
	assert(bits32 == *jive_output_get_type(apply_node->outputs[0]));

	const jive_type * tmparray12[] = {&bits32};
	jive_node * interest = jive_test_node_create(graph->root_region,
		1, tmparray12, apply_node->outputs, 1, tmparray12);
	
	jive_graph_export(graph, interest->outputs[0]);
	
	jive_view(graph, stderr);

	jive_inline_lambda_apply(apply_node);
	jive_graph_prune(graph);
	
	jive_view(graph, stderr);

	jive_node * test_sum = interest->producer(0);
	assert(jive_node_isinstance(test_sum, &JIVE_BITSUM_NODE));
	assert(test_sum->ninputs == 2);
	assert(test_sum->inputs[0]->origin() == c0);
	assert(test_sum->inputs[1]->origin() == c1);
	
	jive_graph_destroy(graph);
	jive_context_assert_clean(ctx);
	jive_context_destroy(ctx);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("function/test-lambda-apply", test_main);
