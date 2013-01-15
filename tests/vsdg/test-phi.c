/*
 * Copyright 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */
#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/phi.h>
#include <jive/types/function/fctlambda.h>
#include <jive/types/function/fctapply.h>
#include <jive/vsdg/node-private.h>

static int test_main()
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	JIVE_DECLARE_VALUE_TYPE(vtype);
	jive_function_type * f0type = jive_function_type_create(context, 0, NULL, 0, NULL);
	jive_function_type * f1type = jive_function_type_create(context, 0, NULL, 0, NULL);	
	jive_function_type * f2type = jive_function_type_create(context, 1, &vtype,
		1, (const jive_type *[]){vtype});

	jive_output * arguments[3];
	jive_region * phi_region = jive_phi_region_create(graph->root_region,
		3, (const jive_type *[]){&f0type->base.base, &f1type->base.base, &f2type->base.base},
		arguments);

	jive_region * lambda_region0 = jive_function_region_create(phi_region);
	jive_region * lambda_region1 = jive_function_region_create(phi_region);
	jive_region * lambda_region2 = jive_function_region_create(phi_region);

	jive_gate * arg_gate = jive_type_create_gate(vtype, graph, "arg");
	jive_gate * ret0_gate = jive_type_create_gate(vtype, graph, "ret0");
	jive_output * arg = jive_node_gate_output(jive_region_get_top_node(lambda_region2), arg_gate);
	jive_output * ret;
	jive_apply_create(arguments[2], 1, &arg, &ret);
	jive_node_gate_input(jive_region_get_bottom_node(lambda_region2), ret0_gate, ret);	

	jive_output * lambda0 = jive_lambda_create(lambda_region0);	
	jive_output * lambda1 = jive_lambda_create(lambda_region1);
	jive_output * lambda2 = jive_lambda_create(lambda_region2);

	jive_phi_region_finalize(phi_region, 3, (jive_output *[]){lambda0, lambda1, lambda2});

	jive_output * results[3];
	jive_phi_create(phi_region, results);

	jive_node * bottom = jive_node_create(graph->root_region,
		3, (const jive_type *[]){&f0type->base.base, &f1type->base.base, &f2type->base.base}, results,
		0, NULL);
	jive_node_reserve(bottom);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);

	jive_view(graph, stderr);

	jive_function_type_destroy(f0type);
	jive_function_type_destroy(f1type);
	jive_function_type_destroy(f2type);
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-phi", test_main);