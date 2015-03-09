/*
 * Copyright 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testnodes.h"
#include "testtypes.h"

#include <assert.h>
#include <locale.h>

#include <jive/types/bitstring.h>
#include <jive/types/function.h>
#include <jive/types/function/fctapply.h>
#include <jive/types/function/fctlambda.h>
#include <jive/types/function/fctsymbolic.h>
#include <jive/types/function/fcttype.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/phi.h>

static void
test_simple_lambda(struct jive_graph * graph)
{
	jive_test_value_type vtype;
	const jive::base::type * tmparray0[] = {&vtype, &vtype, &vtype};
	jive_node * top = jive_test_node_create(graph->root_region, 0, NULL, NULL, 3,
		tmparray0);
	const jive::base::type * tmparray1[] = {&vtype, &vtype, &vtype};
	const char * tmparray2[] = {"x", "y", "z"};

	jive_lambda * lambda = jive_lambda_begin(graph->root_region,
		3, tmparray1, tmparray2);

	jive_node * node = jive_test_node_create(lambda->region, 1, tmparray0, &lambda->arguments[0], 1,
		tmparray0);
	const jive::base::type * tmparray3[] = {&vtype, &vtype};
	jive::output * tmparray4[] = {node->outputs[0], lambda->arguments[1]};

	jive::output * fct = jive_lambda_end(lambda, 2, tmparray3,
		tmparray4);

	std::vector<jive::output *> results = jive_apply_create(fct, 3, &top->outputs[0]);
	const jive::base::type * tmparray5[] = {&vtype, &vtype};

	jive_node * bottom = jive_test_node_create(graph->root_region,
		2, tmparray5, &results[0],
		1, tmparray0);
	jive_graph_export(graph, bottom->outputs[0]);

	jive_view(graph, stderr);

	jive_lambda_node_remove_dead_parameters(fct->node());
	jive_graph_prune(graph);

	jive_view(graph, stderr);

	assert(bottom->producer(1) == top);

	jive_node * apply = bottom->producer(0);
	assert(apply->ninputs == 2);

}

static void
test_recursive_lambda(struct jive_graph * graph)
{
	jive_test_value_type vtype;
	const jive::base::type * tmparray6[] = {&vtype, &vtype, &vtype};
	jive_node * top = jive_test_node_create(graph->root_region, 0, NULL, NULL, 3,
		tmparray6);
	const jive::base::type * tmparray7[] = {&vtype, &vtype, &vtype};
	const jive::base::type * tmparray8[] = {&vtype, &vtype};

	jive::fct::type fcttype(3, tmparray7, 2, tmparray8);

	jive_phi phi = jive_phi_begin(graph->root_region);
	jive_phi_fixvar fv = jive_phi_fixvar_enter(phi, &fcttype);
	const jive::base::type * tmparray9[] = {&vtype, &vtype, &vtype};
	const char * tmparray10[] = {"x", "y", "z"};

	jive_lambda * lambda = jive_lambda_begin(phi.region,
		3, tmparray9, tmparray10);

	jive_node * node = jive_test_node_create(lambda->region, 1, tmparray6, &lambda->arguments[0], 1,
		tmparray6);

	jive::output * tmparray11[] = {node->outputs[0], node->outputs[0], node->outputs[0]};
	std::vector<jive::output *> results = jive_apply_create(fv.value, 3, tmparray11);
	const jive::base::type * tmparray12[] = {&vtype, &vtype};
	jive::output * tmparray13[] = {results[0], lambda->arguments[1]};

	jive::output * fct = jive_lambda_end(lambda, 2,
		tmparray12, tmparray13);

	jive_phi_fixvar_leave(phi, fv.gate, fct);
	jive_phi_end(phi, 1, &fv);

	results = jive_apply_create(fv.value, 3, &top->outputs[0]);
	const jive::base::type * tmparray14[] = {&vtype, &vtype};

	jive_node * bottom = jive_test_node_create(graph->root_region,
		2, tmparray14, &results[0],
		1, tmparray14);
	jive_graph_export(graph, bottom->outputs[0]);

	jive_view(graph, stderr);

	jive_lambda_node_remove_dead_parameters(fct->node());
	jive_graph_prune(graph);

	jive_view(graph, stderr);
}

static int
function_test_lambda_dead_parameter_removal(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();
	test_simple_lambda(graph);
	jive_graph_destroy(graph);

	graph = jive_graph_create();
	test_recursive_lambda(graph);
	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/function/test-lambda_dead_parameter_removal",
	function_test_lambda_dead_parameter_removal);

static int function_test_build_lambda(void)
{
	setlocale(LC_ALL, "");
	jive_graph * graph = jive_graph_create();

	jive::bits::type bits32(32);
	const jive::base::type * tmparray0[] = {&bits32, &bits32};
	const char * tmparray1[] = {"arg1", "arg2"};
	jive_lambda * lambda = jive_lambda_begin(graph->root_region,
		2, tmparray0, tmparray1);

	jive::output * sum = jive_bitsum(lambda->narguments, lambda->arguments);

	jive::output * fct = jive_lambda_end(lambda, 1, tmparray0, &sum);
	
	jive_view(graph, stderr);
	
	const jive::base::type * tmparray2[] = {&bits32, &bits32};
	jive::fct::type ftype(2, tmparray2, 1, tmparray2);

	assert(ftype == fct->type());
	
	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("function/test-build-lambda", function_test_build_lambda);

static int function_test_call(void)
{
	setlocale( LC_ALL, "" ) ;

	jive_graph* graph = jive_graph_create();

	jive::bits::type btype(8);
	const jive::base::type*  tmparray0[] = { &btype };
	const jive::base::type*  tmparray1[] = { &btype };
	jive::fct::type ftype(1, tmparray0, 1, tmparray1) ;

	jive::output* constant = jive_bitconstant( graph, 8, "00001111" ) ;
	jive::output* func = jive_symbolicfunction_create( graph, "sin", &ftype ) ;
	jive::output*  tmparray2[] = { constant };
	jive::output * ret = jive_apply_create(func, 1, tmparray2)[0];

	assert(ret->type() == btype);

	jive_view( graph, stderr ) ;

	jive_graph_destroy( graph ) ;
 
	return 0 ;
}

JIVE_UNIT_TEST_REGISTER("function/test-call", function_test_call);

static int function_test_equals(void)
{
	setlocale( LC_ALL, "" ) ;

	jive::bits::type btype0(8);
	jive::bits::type btype1(8);
	const jive::base::type*  tmparray0[] = { &btype0 };
	const jive::base::type*  tmparray1[] = { &btype0 };

	jive::fct::type type0(1, tmparray0, 1, tmparray1);

	const jive::base::type*  tmparray2[] = { &btype0 };
	const jive::base::type*  tmparray3[] = { &btype1 };
	jive::fct::type type1(1, tmparray2, 1, tmparray3);

	const jive::base::type*  tmparray4[] = { &btype0 };
	const jive::base::type*  tmparray5[] = { &btype1, &btype1 };
	jive::fct::type type2(1, tmparray4, 2, tmparray5);

	const jive::base::type*  tmparray6[] = { &btype0, &btype0 };
	const jive::base::type*  tmparray7[] = { &btype0 };
	jive::fct::type type3(2, tmparray6, 1, tmparray7);

	assert(type0 == type0);
	assert(type0 == type1);
	assert(type0 != type2);
	assert(type0 != type3);
	
	return 0 ;
}

JIVE_UNIT_TEST_REGISTER("function/test-equals", function_test_equals);

static int function_test_lambda_apply(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();
	
	jive::bits::type bits32(32);
	const jive::base::type * tmparray0[] = {&bits32, &bits32};
	const char * tmparray1[] = {"arg1", "arg2"};
	jive_lambda * lambda = jive_lambda_begin(graph->root_region,
		2, tmparray0, tmparray1);

	jive::output * sum = jive_bitsum(lambda->narguments, lambda->arguments);

	const jive::base::type * tmparray11[] = {&bits32};
	jive::output * lambda_expr = jive_lambda_end(lambda, 1, tmparray11, &sum);
	
	jive::output * c0 = jive_bitconstant(graph, 32, "01010100000000000000000000000000");
	jive::output * c1 = jive_bitconstant(graph, 32, "10010010000000000000000000000000");
	jive::output * tmparray2[] = {c0, c1};
	
	jive::output * apply_results[1] = {
		jive_apply_create(lambda_expr, 2, tmparray2)[0]
	};
	
	const jive::base::type * tmparray12[] = {&bits32};
	jive_node * interest = jive_test_node_create(graph->root_region,
		1, tmparray12, apply_results, 1, tmparray12);
	
	jive_graph_export(graph, interest->outputs[0]);
	
	jive_view(graph, stderr);

	jive_inline_lambda_apply(apply_results[0]->node());
	jive_graph_prune(graph);
	
	jive_view(graph, stderr);

	jive_node * test_sum = interest->producer(0);
	assert(test_sum->operation() == jive::bits::add_op(32));
	assert(test_sum->ninputs == 2);
	assert(test_sum->inputs[0]->origin() == c0);
	assert(test_sum->inputs[1]->origin() == c1);
	
	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("function/test-lambda-apply", function_test_lambda_apply);

static int function_test_memory_leak(void)
{
	jive_test_value_type value_type;
	const jive::base::type * value_type_ptr = &value_type;
	jive::fct::type t1(1, &value_type_ptr, 1, &value_type_ptr);

	const jive::base::type * tmparray2[] = {&t1};
	const jive::base::type * tmparray3[] = {&t1};
	jive::fct::type t2(1, tmparray2, 1, tmparray3);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("function/test-memory-leak", function_test_memory_leak);
