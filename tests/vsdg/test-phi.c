/*
 * Copyright 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>
#include <locale.h>

#include <jive/types/function/fctapply.h>
#include <jive/types/function/fctlambda.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/phi.h>
#include <jive/vsdg/seqtype.h>

#include "testnodes.h"

static int test_main()
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive_test_value_type vtype;
	jive::fct::type f0type(0, NULL, 0, NULL);
	const jive::base::type * tmparray0[] = {&vtype};
	jive::fct::type f1type(0, NULL, 0, NULL);
	jive::fct::type f2type(1, tmparray0, 1, tmparray0);

	jive_phi phi = jive_phi_begin(graph->root_region);
	jive_phi_fixvar fns[3];
	fns[0] = jive_phi_fixvar_enter(phi, &f0type);
	fns[1] = jive_phi_fixvar_enter(phi, &f1type);
	fns[2] = jive_phi_fixvar_enter(phi, &f2type);

	jive_lambda * l0 = jive_lambda_begin(phi.region, 0, NULL, NULL);
	jive::output * lambda0 = jive_lambda_end(l0, 0, NULL, NULL);

	jive_lambda * l1 = jive_lambda_begin(phi.region, 0, NULL, NULL);
	jive::output * lambda1 = jive_lambda_end(l1, 0, NULL, NULL);

	const char * tmparray1[] = {"arg"};
	jive_lambda * l2 = jive_lambda_begin(phi.region, 1, tmparray0, tmparray1);
	jive::fct::lambda_dep depvar = jive::fct::lambda_dep_add(l2, fns[2].value);
	jive::output * ret = jive_apply_create(depvar.output, 1, l2->arguments)[0];
	jive::output * lambda2 = jive_lambda_end(l2, 1, tmparray0, &ret);

	jive_phi_fixvar_leave(phi, fns[0].gate, lambda0);
	jive_phi_fixvar_leave(phi, fns[1].gate, lambda1);
	jive_phi_fixvar_leave(phi, fns[2].gate, lambda2);

	jive_phi_end(phi, 3, fns);

	jive::output * results[3] = {fns[0].value, fns[1].value, fns[2].value};

	jive_node * bottom = jive_test_node_create(graph->root_region,
		{&f0type, &f1type, &f2type}, {results[0], results[1], results[2]}, {&vtype});
	jive_graph_export(graph, bottom->output(0));

	jive_graph_normalize(graph);
	jive_graph_prune(graph);

	jive_view(graph, stderr);

	jive_node * lambda_node2;
	lambda_node2 = dynamic_cast<jive::output*>(phi.region->bottom->input(3)->origin())->node();
	assert(jive_lambda_is_self_recursive(lambda_node2));
	assert(dynamic_cast<const jive::seq::type*>(&phi.region->bottom->input(0)->type()));

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-phi", test_main);
