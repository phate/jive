/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <locale.h>
#include <assert.h>

#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/types/function/fctlambda.h>
#include <jive/types/bitstring.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	JIVE_DECLARE_BITSTRING_TYPE(bits32, 32);
	const char * tmparray0[] = {"arg"};
	jive_lambda * lambda = jive_lambda_begin(graph, 1, &bits32, tmparray0);

	jive_output * c0 = jive_bitconstant_unsigned(graph, 32, 3);
	jive_output * c1 = jive_bitconstant_unsigned(graph, 32, 4);
	
	jive_node_normal_form * sum_nf = jive_graph_get_nodeclass_form(graph, &JIVE_BITSUM_NODE);
	assert(sum_nf);
	jive_node_normal_form_set_mutable(sum_nf, false);
jive_output * tmparray1[] = {lambda->arguments[0], c0};

	jive_output * sum0 = jive_bitsum(2, tmparray1);
	assert(jive_node_isinstance(sum0->node, &JIVE_BITSUM_NODE));
	assert(sum0->node->noperands == 2);
	jive_output * tmparray2[] = {sum0, c1};
	
	jive_output * sum1 = jive_bitsum(2, tmparray2);
	assert(jive_node_isinstance(sum1->node, &JIVE_BITSUM_NODE));
	assert(sum1->node->noperands == 2);

	jive_node * lambda_node = jive_lambda_end(lambda, 1, &bits32, &sum1)->node;
	jive_input * retval = lambda_node->inputs[0]->origin->node->inputs[1];
	jive_output * arg = lambda_node->inputs[0]->origin->node->inputs[0]->origin->node->outputs[1];
	jive_graph_export(graph, lambda_node->outputs[0]);
	
	jive_node_normal_form_set_mutable(sum_nf, true);
	jive_graph_normalize(graph);
	jive_graph_prune(graph);
	
	jive_output * expected_sum = retval->origin;
	assert(jive_node_isinstance(expected_sum->node, &JIVE_BITSUM_NODE));
	assert(expected_sum->node->noperands == 2);
	jive_output * op1 = expected_sum->node->inputs[0]->origin;
	jive_output * op2 = expected_sum->node->inputs[1]->origin;
	if (!jive_node_isinstance(op1->node, &JIVE_BITCONSTANT_NODE)) {
		jive_output * tmp = op1; op1 = op2; op2 = tmp;
	}
	assert(jive_node_isinstance(op1->node, &JIVE_BITCONSTANT_NODE));
	assert(jive_bitconstant_equals_unsigned((jive_bitconstant_node *) op1->node, 3+4));
	assert(op2 == arg);

	jive_view(graph, stdout);

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/test-normalize", test_main);
