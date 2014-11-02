/*
 * Copyright 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/arch/registers.h>
#include <jive/regalloc.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/theta.h>

#include "testarch.h"

static jive_graph *
create_testgraph()
{
	/* requires post-op transfer to satisfy register constraints */
	jive_graph * graph = jive_graph_create();
	const jive_argument_type tmparray0[] = { jive_argument_int, jive_argument_int, jive_argument_int };
	
	jive_subroutine subroutine = jive_testarch_subroutine_begin(
		graph,
		3, tmparray0,
		0, NULL
	);
	jive::output * memstate = jive_subroutine_simple_get_global_state(subroutine);
	const jive::base::type * memtype = &memstate->type();
	jive_node * enter_mux = jive_state_split(memtype, memstate, 1)[0]->node();
	jive_node * leave_mux = jive_state_merge(memtype, 1, &enter_mux->outputs[0])->node();
	jive_subroutine_simple_set_global_state(subroutine, leave_mux->outputs[0]);
	
	jive::output * arg1 = jive_subroutine_simple_get_argument(subroutine, 0);
	jive::output * arg2 = jive_subroutine_simple_get_argument(subroutine, 1);
	jive::output * arg3 = jive_subroutine_simple_get_argument(subroutine, 2);
	
	jive_theta theta = jive_theta_begin(graph);
	jive_theta_loopvar loopvar1 = jive_theta_loopvar_enter(theta, arg1);
	jive_theta_loopvar loopvar2 = jive_theta_loopvar_enter(theta, arg2);

	jive_region * theta_region = theta.region;
	jive::output * tmparray1[] = {loopvar1.value, arg3};
	
	jive::output * val = jive_instruction_node_create(
		theta_region,
		&jive_testarch_instr_add_gpr,
		tmparray1, NULL)->outputs[0];
	jive::output * tmparray2[] = {val};
	
	jive::output * ctl = jive_instruction_node_create(
		theta_region,
		&jive_testarch_instr_jumpz,
		tmparray2, NULL)->outputs[0];
	
	jive_theta_loopvar_leave(theta, loopvar1.gate, val);
	jive_theta_loopvar_leave(theta, loopvar2.gate, val);
	
	jive_theta_end(theta, ctl, 1, &loopvar1);
	jive::output * retval = loopvar1.value;
	jive::gate * retval_gate = jive_register_class_create_gate(&jive_testarch_regcls_r3, graph,
		"retval");
	jive_node_gate_input(leave_mux, retval_gate, retval);
	
	jive_graph_export(graph, jive_subroutine_end(subroutine)->outputs[0]);
	
	return graph;
}

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_graph * graph = create_testgraph();
	
	jive_view(graph, stdout);
	
	jive_shaped_graph * shaped_graph = jive_regalloc(graph);
	
	jive_view(graph, stdout);
	
	jive_shaped_graph_destroy(shaped_graph);
	jive_graph_destroy(graph);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("regalloc/test-loop-overflow", test_main);
