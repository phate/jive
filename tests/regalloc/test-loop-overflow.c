#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <jive/arch/registers.h>
#include <jive/context.h>
#include <jive/regalloc.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/theta.h>
#include <jive/vsdg.h>
#include <jive/view.h>

#include "testarch.h"

static jive_graph *
create_testgraph(jive_context * context)
{
	/* requires post-op transfer to satisfy register constraints */
	jive_graph * graph = jive_graph_create(context);
	
	jive_subroutine subroutine = jive_testarch_subroutine_begin(
		graph,
		3, (const jive_argument_type[]) { jive_argument_int, jive_argument_int, jive_argument_int },
		0, NULL
	);
	jive_output * memstate = jive_subroutine_simple_get_global_state(subroutine);
	const jive_type * memtype = jive_output_get_type(memstate);
	jive_node * enter_mux = jive_state_split(memtype, memstate, 1);
	jive_node * leave_mux = jive_state_merge(memtype, 1, enter_mux->outputs)->node;
	jive_subroutine_simple_set_global_state(subroutine, leave_mux->outputs[0]);
	
	jive_output * arg1 = jive_subroutine_simple_get_argument(subroutine, 0);
	jive_output * arg2 = jive_subroutine_simple_get_argument(subroutine, 1);
	jive_output * arg3 = jive_subroutine_simple_get_argument(subroutine, 2);
	
	jive_theta theta = jive_theta_begin(graph);
	jive_theta_loopvar loopvar1 = jive_theta_loopvar_enter(theta, arg1);
	jive_theta_loopvar loopvar2 = jive_theta_loopvar_enter(theta, arg2);

	jive_region * theta_region = theta.region;
	
	jive_output * val = jive_instruction_node_create(
		theta_region,
		&jive_testarch_instr_add_gpr,
		(jive_output *[]) {loopvar1.value, arg3}, NULL)->outputs[0];
	
	jive_output * ctl = jive_instruction_node_create(
		theta_region,
		&jive_testarch_instr_jumpz,
		(jive_output *[]) {val}, NULL)->outputs[0];
	
	jive_theta_loopvar_leave(theta, loopvar1.gate, val);
	jive_theta_loopvar_leave(theta, loopvar2.gate, val);
	
	jive_theta_end(theta, ctl, 1, &loopvar1);
	jive_output * retval = loopvar1.value;
	jive_gate * retval_gate = jive_register_class_create_gate(&jive_testarch_regcls_r3, graph, "retval");
	jive_node_gate_input(leave_mux, retval_gate, retval);
	
	jive_graph_export(graph, jive_subroutine_end(subroutine)->outputs[0]);
	
	return graph;
}

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	
	jive_graph * graph = create_testgraph(ctx);
	
	jive_view(graph, stdout);
	
	jive_shaped_graph * shaped_graph = jive_regalloc(graph);
	
	jive_view(graph, stdout);
	
	jive_shaped_graph_destroy(shaped_graph);
	jive_graph_destroy(graph);
	
	jive_context_assert_clean(ctx);
	jive_context_destroy(ctx);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("regalloc/test-loop-overflow", test_main);
