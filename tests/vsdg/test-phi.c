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

#include "testnodes.h"

static int test_main()
{
	jive::graph graph;

	jive::test::valuetype vtype;
	jive::fct::type f0type((const std::vector<const jive::base::type*>){}, {});
	jive::fct::type f1type({&vtype}, {&vtype});

	jive_phi phi = jive_phi_begin(graph.root());
	jive_phi_fixvar fns[3];
	fns[0] = jive_phi_fixvar_enter(phi, &f0type);
	fns[1] = jive_phi_fixvar_enter(phi, &f0type);
	fns[2] = jive_phi_fixvar_enter(phi, &f1type);

	jive::lambda_builder lb;
	lb.begin(phi.region, f0type);
	auto lambda0 = lb.end({})->output(0);

	lb.begin(phi.region, f0type);
	auto lambda1 = lb.end({})->output(0);

	lb.begin(phi.region, f1type);
	auto dep = lb.add_dependency(fns[2].value);
	jive::oport * arg[1] = {lb.region()->argument(0)};
	auto ret = jive_apply_create(dep, 1, arg)[0];
	auto lambda2 = lb.end({ret})->output(0);

	jive_phi_fixvar_leave(phi, fns[0].gate, lambda0);
	jive_phi_fixvar_leave(phi, fns[1].gate, lambda1);
	jive_phi_fixvar_leave(phi, fns[2].gate, lambda2);

	jive_phi_end(phi, 3, fns);

	jive::oport * results[3] = {fns[0].value, fns[1].value, fns[2].value};

	auto bottom = jive::test::simple_node_create(graph.root(),
		{&f0type, &f0type, &f1type}, {results[0], results[1], results[2]}, {&vtype});
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
