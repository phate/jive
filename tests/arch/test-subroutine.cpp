/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.hpp"

#include <assert.h>

#include <jive/arch/subroutine/nodes.hpp>
#include <jive/view.hpp>
#include "testarch.hpp"

static int test_main(void)
{
#if 0
	jive::graph graph;
	jive_argument_type  tmparray0[] = {
		jive_argument_long,
		jive_argument_long,
		jive_argument_long,
		jive_argument_long
	};
	jive_argument_type  tmparray1[] = {jive_argument_long};
	
	jive_subroutine subroutine = jive_testarch_subroutine_begin(&graph,
		4, tmparray0,
		1, tmparray1);
	
	auto arg1 = jive_subroutine_simple_get_argument(subroutine, 0);
	auto arg2 = jive_subroutine_simple_get_argument(subroutine, 1);
	auto arg3 = jive_subroutine_simple_get_argument(subroutine, 2);

	auto s1 = jive::create_instruction(subroutine.region, &jive::testarch::instr_add_gpr::instance(),
		{arg1, arg2})->output(0);
	auto s2 = jive::create_instruction(subroutine.region, &jive::testarch::instr_add_gpr::instance(),
		{s1, arg3})->output(0);
	jive_subroutine_simple_set_result(subroutine, 0, dynamic_cast<jive::simple_output*>(s2));
	
	graph.add_export(jive_subroutine_end(subroutine)->output(0), "dummy");
	
	jive::view(graph.root(), stdout);

	jive_context * context2 = jive_context_create();
	// FIXME: copying of subroutine nodes is currently quite broken;
	// reactivate when repaired
	jive::graph * &graph2 = jive_graph_copy(&graph, context2);
	
	jive_subroutine_node * anchor2 = dynamic_cast<jive_subroutine_node *>(
		&graph2->bottom.first->&graph_bottom_list.next);
	assert(anchor2);
	jive_subroutine_deprecated * sub2 = anchor2->operation().subroutine();
	assert(sub2);
	assert(sub2->nparameters == 4);
	assert(sub2->parameters[0] && jive_node_get_gate_output(sub2->enter, sub2->parameters[0]));
	assert(sub2->parameters[1] && jive_node_get_gate_output(sub2->enter, sub2->parameters[1]));
	assert(sub2->parameters[2] && jive_node_get_gate_output(sub2->enter, sub2->parameters[2]));
	assert(sub2->parameters[3]);
	assert(jive_node_get_gate_output(sub2->enter, sub2->parameters[3]) == NULL);
	
	jive_view(&graph2, stdout);
	
	jive_graph_destroy(&graph2);
	assert(jive_context_is_empty(context2));
	jive_context_destroy(context2);
#endif

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-subroutine", test_main)
