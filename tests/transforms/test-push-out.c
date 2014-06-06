/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
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
	
	jive::bits::type int32(32);
	const char * tmparray0[] = {"arg"};

	const jive::base::type * tmparray11[] = {&int32};
	jive_lambda * outer_function = jive_lambda_begin(graph, 1, tmparray11, tmparray0);
	const char * tmparray1[] = {"arg1"};
	
	jive_lambda * inner_function = jive_lambda_begin(graph, 1, tmparray11, tmparray1);
	jive_output * tmparray2[] = {inner_function->arguments[0],
		outer_function->arguments[0]};

	jive_output * sum = jive_bitsum(2, tmparray2);

	jive_node * inner_lambda = jive_lambda_end(inner_function, 1, tmparray11, &sum)->node();
	
	jive_node * apply = jive_apply_node_create(outer_function->region, inner_lambda->outputs[0],
		1, outer_function->arguments);
	
	jive_node * outer_lambda = jive_lambda_end(outer_function, 1, tmparray11,
		&apply->outputs[0])->node();
	jive_graph_export(graph, outer_lambda->outputs[0]);
	
	jive_view(graph, stderr);

	sum->node()->inputs[1]->divert_origin(sum->node()->inputs[0]->origin());

	jive_view(graph, stderr);
	
	assert(jive_node_can_move_outward(inner_lambda));
	jive_graph_push_outward(graph);
	assert(inner_lambda->region == graph->root_region);
	assert(inner_lambda->producer(0)->region->parent == graph->root_region);
	
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
