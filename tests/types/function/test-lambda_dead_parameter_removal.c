/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <locale.h>

#include <jive/types/function.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/phi.h>

#include "testnodes.h"

static void
test_simple_lambda(struct jive_graph * graph)
{
	jive_test_value_type vtype;
	const jive_type * tmparray0[] = {&vtype, &vtype, &vtype};
	jive_node * top = jive_test_node_create(graph->root_region, 0, NULL, NULL, 3,
		tmparray0);
	const jive_type * tmparray1[] = {&vtype, &vtype, &vtype};
	const char * tmparray2[] = {"x", "y", "z"};

	jive_lambda * lambda = jive_lambda_begin(graph,
		3, tmparray1, tmparray2);

	jive_node * node = jive_test_node_create(lambda->region, 1, tmparray0, &lambda->arguments[0], 1,
		tmparray0);
	const jive_type * tmparray3[] = {&vtype, &vtype};
	jive_output * tmparray4[] = {node->outputs[0], lambda->arguments[1]};

	jive_output * fct = jive_lambda_end(lambda, 2, tmparray3,
		tmparray4);

	jive_output * results[2];
	jive_apply_create(fct, 3, top->outputs, results);
	const jive_type * tmparray5[] = {&vtype, &vtype};

	jive_node * bottom = jive_test_node_create(graph->root_region,
		2, tmparray5, results,
		1, tmparray0);
	jive_graph_export(graph, bottom->outputs[0]);

	jive_view(graph, stderr);

	jive_lambda_node_remove_dead_parameters(jive_lambda_node_cast(fct->node));
	jive_graph_prune(graph);

	jive_view(graph, stderr);

	assert(bottom->inputs[1]->origin()->node == top);

	jive_node * apply = bottom->inputs[0]->origin()->node;
	assert(apply->ninputs == 2);

}

static void
test_recursive_lambda(struct jive_graph * graph)
{
	jive_test_value_type vtype;
	const jive_type * tmparray6[] = {&vtype, &vtype, &vtype};
	jive_node * top = jive_test_node_create(graph->root_region, 0, NULL, NULL, 3,
		tmparray6);
	const jive_type * tmparray7[] = {&vtype, &vtype, &vtype};
	const jive_type * tmparray8[] = {&vtype, &vtype};

	jive_function_type fcttype(3, tmparray7, 2, tmparray8);

	jive_phi phi = jive_phi_begin(graph);
	jive_phi_fixvar fv = jive_phi_fixvar_enter(phi, &fcttype);
	const jive_type * tmparray9[] = {&vtype, &vtype, &vtype};
	const char * tmparray10[] = {"x", "y", "z"};

	jive_lambda * lambda = jive_lambda_begin(graph,
		3, tmparray9, tmparray10);

	jive_node * node = jive_test_node_create(lambda->region, 1, tmparray6, &lambda->arguments[0], 1,
		tmparray6);

	jive_output * results[2];
	jive_output * tmparray11[] = {node->outputs[0], node->outputs[0], node->outputs[0]};
	jive_apply_create(fv.value, 3,
		tmparray11, results);
	const jive_type * tmparray12[] = {&vtype, &vtype};
	jive_output * tmparray13[] = {results[0], lambda->arguments[1]};

	jive_output * fct = jive_lambda_end(lambda, 2,
		tmparray12, tmparray13);

	jive_phi_fixvar_leave(phi, fv.gate, fct);
	jive_phi_end(phi, 1, &fv);

	jive_apply_create(fv.value, 3, top->outputs, results);
	const jive_type * tmparray14[] = {&vtype, &vtype};

	jive_node * bottom = jive_test_node_create(graph->root_region,
		2, tmparray14, results,
		1, tmparray14);
	jive_graph_export(graph, bottom->outputs[0]);

	jive_view(graph, stderr);

	jive_lambda_node_remove_dead_parameters(jive_lambda_node_cast(fct->node));
	jive_graph_prune(graph);

	jive_view(graph, stderr);
}

static int
test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();

	jive_graph * graph = jive_graph_create(context);
	test_simple_lambda(graph);
	jive_graph_destroy(graph);

	graph = jive_graph_create(context);
	test_recursive_lambda(graph);
	jive_graph_destroy(graph);

	jive_context_assert_clean(context);
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/function/test-lambda_dead_parameter_removal", test_main);
