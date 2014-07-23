/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/arch/subroutine/nodes.h>
#include <jive/view.h>
#include "testarch.h"

static int test_main(void)
{
	setlocale(LC_ALL, "");
	
	jive_context * context = jive_context_create();
	
	jive_graph * graph = jive_graph_create(context);
	jive_argument_type  tmparray0[] = {
		jive_argument_long,
		jive_argument_long,
		jive_argument_long,
		jive_argument_long
	};
	jive_argument_type  tmparray1[] = {jive_argument_long};
	
	jive_subroutine subroutine = jive_testarch_subroutine_begin(graph,
		4, tmparray0,
		1, tmparray1);
	
	jive::output * arg1 = jive_subroutine_simple_get_argument(subroutine, 0);
	jive::output * arg2 = jive_subroutine_simple_get_argument(subroutine, 1);
	jive::output * arg3 = jive_subroutine_simple_get_argument(subroutine, 2);
	jive::output * tmparray2[] = {arg1, arg2};
	
	jive::output * s1 = jive_instruction_node_create(
		subroutine.region,
		&jive_testarch_instr_add,
		tmparray2, NULL)->outputs[0];
	jive::output * tmparray3[] = {s1, arg3};
	jive::output * s2 = jive_instruction_node_create(
		subroutine.region,
		&jive_testarch_instr_add,
		tmparray3, NULL)->outputs[0];
	jive_subroutine_simple_set_result(subroutine, 0, s2);
	
	jive_graph_export(graph, jive_subroutine_end(subroutine)->outputs[0]);
	
	jive_view(graph, stdout);
	
	jive_context * context2 = jive_context_create();
#if 0
	// FIXME: copying of subroutine nodes is currently quite broken;
	// reactivate when repaired
	jive_graph * graph2 = jive_graph_copy(graph, context2);
	
	jive_subroutine_node * anchor2 = dynamic_cast<jive_subroutine_node *>(
		graph2->bottom.first->graph_bottom_list.next);
	assert(anchor2);
	jive_subroutine_deprecated * sub2 = anchor2->operation().subroutine();
	assert(sub2);
	assert(sub2->nparameters == 4);
	assert(sub2->parameters[0] && jive_node_get_gate_output(sub2->enter, sub2->parameters[0]));
	assert(sub2->parameters[1] && jive_node_get_gate_output(sub2->enter, sub2->parameters[1]));
	assert(sub2->parameters[2] && jive_node_get_gate_output(sub2->enter, sub2->parameters[2]));
	assert(sub2->parameters[3]);
	assert(jive_node_get_gate_output(sub2->enter, sub2->parameters[3]) == NULL);
	
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);
	
	jive_view(graph2, stdout);
	
	jive_graph_destroy(graph2);
#endif
	assert(jive_context_is_empty(context2));
	jive_context_destroy(context2);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-subroutine", test_main);
