/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/arch/regvalue.h>
#include <jive/arch/subroutine/nodes.h>
#include <jive/types/bitstring.h>
#include <jive/view.h>
#include "testarch.h"

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_graph graph;
	jive_argument_type  tmparray0[] = {
			jive_argument_long, jive_argument_long,
			jive_argument_long, jive_argument_long
		};
	jive_argument_type  tmparray1[] = {jive_argument_long};
	
	jive_subroutine subroutine = jive_testarch_subroutine_begin(
		&graph,
		4, tmparray0,
		1, tmparray1);
	
	jive::output * arg1 = jive_subroutine_simple_get_argument(subroutine, 0);
	
	jive::output * lit = jive_bitconstant_unsigned(subroutine.region, 32, 42);
	jive::output * sym = jive_bitsymbolicconstant(subroutine.region, 32, "symbol");
	jive::output * bitnot = jive_bitnot(sym);
	jive::output* tmparray2[] = {arg1, lit};
	jive::output * sum1 = jive_bitsum(2, tmparray2);
	jive::output* tmparray3[] = {lit, bitnot};
	jive::output * sum2 = jive_bitsum(2, tmparray3);
	jive::output * res = jive_bituquotient(sum1, sum2);
	jive_subroutine_simple_set_result(subroutine, 0, res);
	
	graph.export_port(jive_subroutine_end(subroutine)->output(0), "dummy");
	
	jive_view(&graph, stdout);
	
	jive_testarch_reg_classifier classifier;
	jive_regselector regselect;
	jive_regselector_init(&regselect, &graph, &classifier);
	jive_regselector_process(&regselect);
	jive_regselector_fini(&regselect);
	
	jive_node * n1 = dynamic_cast<jive::output*>(sum1->node()->input(0)->origin())->node();
	jive_node * n2 = dynamic_cast<jive::output*>(sum1->node()->input(1)->origin())->node();
	
	const jive::regvalue_op * rv = dynamic_cast<const jive::regvalue_op *>(
		&n1->operation());
	if (!rv) {
		rv = dynamic_cast<const jive::regvalue_op *>(
			&n2->operation());
	}
	assert(rv);
	assert(rv->regcls() == &jive_testarch_regcls_gpr);
	
	sum2 = dynamic_cast<jive::output*>(res->node()->input(1)->origin());
	n1 = dynamic_cast<jive::output*>(sum2->node()->input(0)->origin())->node();
	n2 = dynamic_cast<jive::output*>(sum2->node()->input(1)->origin())->node();
	assert(dynamic_cast<const jive::regvalue_op *>(&n1->operation()));
	assert(dynamic_cast<const jive::bits::not_op *>(&n2->operation()));
	n2 = dynamic_cast<jive::output*>(n2->input(0)->origin())->node();
	assert(dynamic_cast<const jive::regvalue_op *>(&n2->operation()));
	jive::output * o1 = dynamic_cast<jive::output*>(n1->input(1)->origin());
	jive::output * o2 = dynamic_cast<jive::output*>(n2->input(1)->origin());
	assert(o1 == lit);
	assert(o2 == sym);
	
	jive_graph_prune(&graph);
	jive_view(&graph, stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-regselect", test_main);
