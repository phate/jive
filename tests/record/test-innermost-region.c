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
#include <jive/types/record/rcdgroup.h>
#include <jive/types/record/rcdselect.h>
#include <jive/types/function/fctlambda.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);

	JIVE_DECLARE_BITSTRING_TYPE(int8, 8);
	jive_lambda * lambda = jive_lambda_begin(graph, 1, &int8, (const char *[]){"arg"});

	jive_output * o0 = jive_bitconstant(graph, 8, "00000010");

	static const jive_bitstring_type bits8 = {{{&JIVE_BITSTRING_TYPE}}, 8};
	
	static const jive_value_type * l_elements[] = { &bits8.base, &bits8.base };
	static const jive_record_declaration l = {2, l_elements};

	jive_output * g = jive_group_create(&l, 2, (jive_output * []){o0, lambda->arguments[0]});
	jive_output * s = jive_select_create(1, g);

	jive_node * lambda_node = jive_lambda_end(lambda, 1, &int8, &s)->node;
	jive_node_reserve(lambda_node);

	assert(lambda_node->inputs[0]->origin->node->region == g->node->region);

	jive_view(graph, stderr);

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);

	return 0;	
}

JIVE_UNIT_TEST_REGISTER("record/test-innermost-region", test_main);
