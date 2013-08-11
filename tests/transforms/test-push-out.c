/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
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

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	JIVE_DECLARE_BITSTRING_TYPE(int32, 32);

	jive_lambda * outer_function = jive_lambda_begin(graph, 1, &int32, (const char *[]){"arg"});
	
	jive_lambda * inner_function = jive_lambda_begin(graph, 1, &int32, (const char *[]){"arg1"});

	jive_output * sum = jive_bitsum(2, (jive_output *[]){inner_function->arguments[0],
		outer_function->arguments[0]});

	jive_node * inner_lambda = jive_lambda_end(inner_function, 1, &int32, &sum)->node;
	
	jive_node * apply = jive_apply_node_create(outer_function->region, inner_lambda->outputs[0],
		1, outer_function->arguments);
	
	jive_node * outer_lambda = jive_lambda_end(outer_function, 1, &int32, &apply->outputs[0])->node;
	jive_node_reserve(outer_lambda);
	
	jive_view(graph, stderr);

	jive_input_divert_origin(sum->node->inputs[1], sum->node->inputs[0]->origin);

	jive_view(graph, stderr);
	
	assert(jive_node_can_move_outward(inner_lambda));
	jive_graph_push_outward(graph);
	assert(inner_lambda->region == graph->root_region);
	assert(inner_lambda->inputs[0]->origin->node->region->parent == graph->root_region);
	
	jive_view(graph, stderr);
	
	jive_graph_pull_inward(graph);
	/* must not be pulled back into lambda def region */
	assert(inner_lambda->region == graph->root_region);
	
	jive_view(graph, stderr);
	
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("transforms/test-push-out", test_main);
