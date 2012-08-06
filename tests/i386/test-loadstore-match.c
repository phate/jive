#include "test-registry.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <locale.h>

#include <jive/arch/load.h>
#include <jive/arch/store.h>
#include <jive/arch/regselector.h>
#include <jive/backend/i386/classifier.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/instrmatch.h>
#include <jive/backend/i386/machine.h>
#include <jive/backend/i386/registerset.h>
#include <jive/backend/i386/subroutine.h>
#include <jive/regalloc.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/types/bitstring.h>
#include <jive/util/buffer.h>
#include <jive/vsdg.h>

static void *
compile_graph(jive_graph * graph)
{
	jive_regselector regselector;
	jive_regselector_init(&regselector, graph, &jive_i386_reg_classifier);
	jive_regselector_process(&regselector);
	jive_i386_match_instructions(graph, &regselector);
	jive_regselector_fini(&regselector);
	jive_graph_prune(graph);
	
	jive_shaped_graph * shaped_graph = jive_regalloc(graph);
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_buffer buffer;
	jive_buffer_init(&buffer, graph->context);
	jive_graph_generate_assembler(graph, &buffer);
	fwrite(buffer.data, buffer.size, 1, stderr);
	jive_buffer_fini(&buffer);
	
	jive_buffer_init(&buffer, graph->context);
	jive_graph_generate_code(graph, &buffer);
	
	void * result = jive_buffer_executable(&buffer);
	jive_buffer_fini(&buffer);
	
	jive_graph_destroy(graph);
	
	return result;
}

static jive_graph *
prepare_graph(jive_context * ctx)
{
	jive_graph * graph;
	graph = jive_graph_create(ctx);
	
	jive_subroutine * sub = jive_i386_subroutine_create(
		graph->root_region,
		2, (jive_argument_type []) { jive_argument_int, jive_argument_int },
		0, NULL);
	jive_node_reserve(&sub->subroutine_node->base);
	
	JIVE_DECLARE_STATE_TYPE(state);
	jive_output * state1 = jive_node_add_output(&sub->enter->base, state);
	jive_output * state2 = jive_node_add_output(&sub->enter->base, state);
	
	jive_output * arg1 = jive_subroutine_value_parameter(sub, 0);
	jive_output * arg2 = jive_subroutine_value_parameter(sub, 1);
	
	JIVE_DECLARE_BITSTRING_TYPE(bits32, 32);
	
	jive_output * v1 = jive_load_by_bitstring_create(
		arg1, 32,
		(const jive_value_type *) bits32,
		1, &state1);
	
	jive_output * v2 = jive_load_by_bitstring_create(
		arg2, 32,
		(const jive_value_type *) bits32,
		1, &state2);
	
	jive_output * sum = jive_bitsum(2, (jive_output *[]){v1, v2});
	jive_output * diff = jive_bitdifference(v1, v2);

	jive_output * state_;	
	jive_store_by_bitstring_create(
		arg1, 32,
		(const jive_value_type *) bits32, sum,
		1, &state1, &state_);
	state1 = state_;
	
	jive_store_by_bitstring_create(
		arg2, 32,
		(const jive_value_type *) bits32, diff,
		1, &state2, &state_);
	state2 = state_;
	
	jive_node_add_input(&sub->leave->base, state, state1);
	jive_node_add_input(&sub->leave->base, state, state2);
	
	return graph;
}

typedef void (*testfn_t)(int *, int *);

static int test_main(void)
{
	setlocale(LC_ALL, "");
	
	jive_context * ctx = jive_context_create();
	
	jive_graph * graph = prepare_graph(ctx);
	testfn_t fn = (testfn_t) compile_graph(graph);
	
	int x, y, m, n;
	for (m = -5 ; m < 6; ++m) {
		for (n = -5; n < 6; ++n) {
			x = m;
			y = n;
			fn(&x, &y);
			assert(x == m + n);
			assert(y == m - n);
		}
	}
	
	assert(jive_context_is_empty(ctx));
	
	jive_context_destroy(ctx);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("i386/test-loadstore-match", test_main);
