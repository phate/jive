/*
 * Copyright 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>

#include <jive/rvsdg.h>
#include <jive/rvsdg/phi.h>
#include <jive/types/function/fctapply.h>
#include <jive/types/function/fctlambda.h>
#include <jive/view.h>

#include "testnodes.h"

static int test_main()
{
	jive::graph graph;

	jive::test::valuetype vtype;
	jive::fct::type f0type((const std::vector<const jive::type*>){}, {});
	jive::fct::type f1type({&vtype}, {&vtype});

	jive::phi_builder pb;
	pb.begin_phi(graph.root());
	auto rv1 = pb.add_recvar(f0type);
	auto rv2 = pb.add_recvar(f0type);
	auto rv3 = pb.add_recvar(f1type);

	jive::lambda_builder lb;
	lb.begin_lambda(pb.region(), f0type);
	auto lambda0 = lb.end_lambda({})->output(0);

	lb.begin_lambda(pb.region(), f0type);
	auto lambda1 = lb.end_lambda({})->output(0);

	auto arguments = lb.begin_lambda(pb.region(), f1type);
	auto dep = lb.add_dependency(rv3->value());
	auto ret = jive::fct::create_apply(dep, {arguments[0]})[0];
	auto lambda2 = lb.end_lambda({ret})->output(0);

	rv1->set_value(lambda0);
	rv2->set_value(lambda1);
	rv3->set_value(lambda2);

	auto phi = pb.end_phi();
	graph.add_export(phi->output(0), "dummy");

	graph.normalize();
	graph.prune();

	jive::view(graph.root(), stderr);

	jive::node * lambda_node2;
	lambda_node2 = phi->subregion(0)->result(2)->origin()->node();
	assert(jive_lambda_is_self_recursive(lambda_node2));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("rvsdg/test-phi", test_main);
