/*
 * Copyright 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>

#include <jive/types/function/fctapply.h>
#include <jive/types/function/fctlambda.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/phi.h>
#include <jive/vsdg/seqtype.h>

#include "testnodes.h"

static int test_main()
{
	jive_graph graph;

	jive_test_value_type vtype;
	jive::fct::type f0type(0, NULL, 0, NULL);
	const jive::base::type * tmparray0[] = {&vtype};
	jive::fct::type f1type(0, NULL, 0, NULL);
	jive::fct::type f2type(1, tmparray0, 1, tmparray0);

	jive_phi phi = jive_phi_begin(graph.root());
	jive_phi_fixvar fns[3];
	fns[0] = jive_phi_fixvar_enter(phi, &f0type);
	fns[1] = jive_phi_fixvar_enter(phi, &f1type);
	fns[2] = jive_phi_fixvar_enter(phi, &f2type);

	jive_lambda * l0 = jive_lambda_begin(phi.region, {}, {});
	auto lambda0 = jive_lambda_end(l0, 0, NULL, NULL);

	jive_lambda * l1 = jive_lambda_begin(phi.region, {}, {});
	auto lambda1 = jive_lambda_end(l1, 0, NULL, NULL);

	jive_lambda * l2 = jive_lambda_begin(phi.region, {{&vtype, "arg"}}, {{&vtype, "r"}});
	jive::fct::lambda_dep depvar = jive::fct::lambda_dep_add(l2, fns[2].value);
	auto ret = jive_apply_create(depvar.output, 1, l2->arguments)[0];
	auto lambda2 = jive_lambda_end(l2, 1, tmparray0, &ret);

	jive_phi_fixvar_leave(phi, fns[0].gate, lambda0);
	jive_phi_fixvar_leave(phi, fns[1].gate, lambda1);
	jive_phi_fixvar_leave(phi, fns[2].gate, lambda2);

	jive_phi_end(phi, 3, fns);

	jive::oport * results[3] = {fns[0].value, fns[1].value, fns[2].value};

	jive::node * bottom = jive_test_node_create(graph.root(),
		{&f0type, &f1type, &f2type}, {results[0], results[1], results[2]}, {&vtype});
	graph.export_port(bottom->output(0), "dummy");

	graph.normalize();
	graph.prune();

	jive::view(graph.root(), stderr);

	jive::node * lambda_node2;
	lambda_node2 = phi.region->result(2)->origin()->node();
	assert(jive_lambda_is_self_recursive(lambda_node2));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-phi", test_main);
