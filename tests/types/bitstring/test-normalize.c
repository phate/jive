/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/types/bitstring.h>
#include <jive/types/function/fctlambda.h>
#include <jive/view.h>
#include <jive/vsdg.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive::bits::type bits32(32);
	const char * tmparray0[] = {"arg"};
	const jive::base::type * tmparray11[] = {&bits32};
	jive_lambda * lambda = jive_lambda_begin(graph, 1, tmparray11, tmparray0);

	jive::output * c0 = jive_bitconstant_unsigned(graph, 32, 3);
	jive::output * c1 = jive_bitconstant_unsigned(graph, 32, 4);
	
	jive::node_normal_form * sum_nf = jive_graph_get_nodeclass_form(
		graph, typeid(jive::bits::add_op));
	assert(sum_nf);
	sum_nf->set_mutable(false);
	jive::output * tmparray1[] = {lambda->arguments[0], c0};

	jive::output * sum0 = jive_bitsum(2, tmparray1);
	assert(sum0->node()->operation() == jive::bits::add_op(32));
	assert(sum0->node()->noperands == 2);
	jive::output * tmparray2[] = {sum0, c1};
	
	jive::output * sum1 = jive_bitsum(2, tmparray2);
	assert(sum1->node()->operation() == jive::bits::add_op(32));
	assert(sum1->node()->noperands == 2);

	jive_node * lambda_node = jive_lambda_end(lambda, 1, tmparray11, &sum1)->node();
	jive::input * retval = lambda_node->producer(0)->inputs[1];
	jive::output * arg = lambda_node->producer(0)->producer(0)->outputs[1];
	jive_graph_export(graph, lambda_node->outputs[0]);
	
	sum_nf->set_mutable(true);
	jive_graph_normalize(graph);
	jive_graph_prune(graph);
	
	jive::output * expected_sum = retval->origin();
	assert(expected_sum->node()->operation() == jive::bits::add_op(32));
	assert(expected_sum->node()->noperands == 2);
	jive::output * op1 = expected_sum->node()->inputs[0]->origin();
	jive::output * op2 = expected_sum->node()->inputs[1]->origin();
	if (!dynamic_cast<const jive::bits::constant_op *>(&op1->node()->operation())) {
		jive::output * tmp = op1; op1 = op2; op2 = tmp;
	}
	assert(op1->node()->operation() == jive::bits::int_constant_op(32, 3 + 4));
	assert(op2 == arg);

	jive_view(graph, stdout);

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/test-normalize", test_main);
