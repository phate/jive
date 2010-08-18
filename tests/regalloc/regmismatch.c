#include <assert.h>
#include <stdio.h>
#include <locale.h>

#include <jive/vsdg.h>
#include <jive/view.h>

#include <jive/regalloc.h>
#include "testarch.h"

static void
proc_frame(jive_context * context, jive_graph ** graph, jive_node ** enter, jive_node ** leave)
{
	*graph = jive_graph_create(context);
	*enter = (jive_node *) jive_instruction_node_create(
		(*graph)->root_region,
		&JIVE_PSEUDO_NOP,
		NULL, NULL);
	*leave = (jive_node *) jive_instruction_node_create(
		(*graph)->root_region,
		&JIVE_PSEUDO_NOP,
		NULL, NULL);
	
	jive_gate * stackptr_var = jive_regcls_create_gate(&jive_testarch_regcls[cls_r4], *graph, "stackptr");
	jive_output * stackptr = jive_node_gate_output(*enter, stackptr_var);
	jive_node_gate_input(*leave, stackptr_var, stackptr);
}

static jive_graph *
create_testgraph_mismatch1(jive_context * context)
{
	jive_graph * graph;
	jive_node * enter, * leave;
	proc_frame(context, &graph, &enter, &leave);
	
	jive_output * arg1 = jive_node_gate_output(enter, jive_regcls_create_gate(&jive_testarch_regcls[cls_r1], graph, "cls1"));
	jive_node_gate_input(leave, jive_regcls_create_gate(&jive_testarch_regcls[cls_r2], graph, "cls2"), arg1);
	
	return graph;
}

typedef jive_graph * (*creator_function_t)(jive_context *);

static const creator_function_t tests[] = {
	create_testgraph_mismatch1
};

int main()
{
	setlocale(LC_ALL, "");
	
	jive_context * context = jive_context_create();
	
	size_t n;
	for(n=0; n<sizeof(tests)/sizeof(tests[0]); n++) {
		jive_graph * graph = tests[n](context);
		jive_view(graph, stdout);
		jive_regalloc(graph, &testarch_transfer_instructions_factory);
		jive_view(graph, stdout);
		jive_graph_destroy(graph);
	}
	
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);
	
	return 0;
}
