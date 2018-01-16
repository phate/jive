/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testarch.h"

#include <assert.h>

#include <jive/arch/regvalue.h>
#include <jive/arch/subroutine/nodes.h>
#include <jive/rvsdg/simple-node.h>
#include <jive/rvsdg/structural-node.h>
#include <jive/types/bitstring.h>
#include <jive/view.h>

static int test_main(void)
{
	using namespace jive;

	jive::graph graph;
	jive_argument_type  tmparray0[] = {
			jive_argument_long, jive_argument_long,
			jive_argument_long, jive_argument_long
		};
	jive_argument_type  tmparray1[] = {jive_argument_long};
	
	jive_subroutine subroutine = jive_testarch_subroutine_begin(
		&graph,
		4, tmparray0,
		1, tmparray1);
	
	auto arg1 = jive_subroutine_simple_get_argument(subroutine, 0);
	
	auto lit = create_bitconstant(subroutine.region, 32, 42);
	auto sym = subroutine.node->subregion(0)->add_argument(nullptr, bittype(32));
	auto bitnot = create_bitnot(32, sym);
	auto sum1 = create_bitadd(32, arg1, lit);
	auto sum2 = create_bitadd(32, lit, bitnot);
	auto res = create_bitudiv(32, sum1, sum2);
	jive_subroutine_simple_set_result(subroutine, 0, dynamic_cast<jive::simple_output*>(res));
	
	graph.add_export(jive_subroutine_end(subroutine)->output(0), "dummy");
	
	jive::view(graph.root(), stdout);
	
	jive_testarch_reg_classifier classifier;
	jive::register_selector regselect(&graph, &classifier);
	jive_regselector_process(&regselect);

/* FIXME: currently broken, comment in again when repaired
	auto tmp1 = dynamic_cast<jive::output*>(sum1);	
	jive::node * n1 = dynamic_cast<jive::output*>(tmp1->node()->input(0)->origin())->node();
	jive::node * n2 = dynamic_cast<jive::output*>(tmp1->node()->input(1)->origin())->node();
	
	const jive::regvalue_op * rv = dynamic_cast<const jive::regvalue_op *>(
		&n1->operation());
	if (!rv) {
		rv = dynamic_cast<const jive::regvalue_op *>(
			&n2->operation());
	}
	assert(rv);
	assert(rv->regcls() == &jive_testarch_regcls_gpr);
	
	jive::output * foo = dynamic_cast<jive::output*>(res->node()->input(1)->origin());
	n1 = dynamic_cast<jive::output*>(foo->node()->input(0)->origin())->node();
	n2 = dynamic_cast<jive::output*>(foo->node()->input(1)->origin())->node();
	assert(dynamic_cast<const jive::regvalue_op *>(&n1->operation()));
	assert(dynamic_cast<const jive::bits::not_op *>(&n2->operation()));
	n2 = dynamic_cast<jive::output*>(n2->input(0)->origin())->node();
	assert(dynamic_cast<const jive::regvalue_op *>(&n2->operation()));
	jive::output * o1 = dynamic_cast<jive::output*>(n1->input(1)->origin());
	jive::output * o2 = dynamic_cast<jive::output*>(n2->input(1)->origin());
	assert(o1 == lit);
	assert(o2 == sym);
	
	graph.prune();
	jive_view(&graph, stdout);
*/
	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-regselect", test_main)
