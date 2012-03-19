#include <assert.h>
#include <stdio.h>
#include <locale.h>

#include <jive/vsdg.h>
#include <jive/view.h>
#include <jive/util/buffer.h>
#include <jive/arch/transfer-instructions.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/registerset.h>
#include <jive/backend/i386/subroutine.h>
#include <jive/backend/i386/machine.h>
#include <jive/backend/i386/classifier.h>
#include <jive/backend/i386/instrmatch.h>
#include <jive/types/bitstring/arithmetic.h>

#include <jive/regalloc.h>
#include <jive/regalloc/shaped-graph.h>

static jive_xfer_block
i386_create_xfer(jive_region * region, jive_output * origin,
	const jive_resource_class * in_class, const jive_resource_class * out_class)
{
	jive_xfer_block xfer;
	
	xfer.node = jive_instruction_node_create(
		region,
		&jive_i386_instructions[jive_i386_int_transfer],
		(jive_output *[]){origin}, NULL);
	xfer.input = xfer.node->inputs[0];
	xfer.output = xfer.node->outputs[0];
	
	return xfer;
}

const jive_transfer_instructions_factory i386_xfer_factory = {
	i386_create_xfer
};

int main()
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_subroutine * subroutine = jive_i386_subroutine_create(graph->root_region,
		2, (jive_argument_type []) { jive_argument_int, jive_argument_int },
		1, (jive_argument_type []) { jive_argument_int });
	
	jive_output * arg1 = jive_subroutine_value_parameter(subroutine, 0);
	jive_output * arg2 = jive_subroutine_value_parameter(subroutine, 1);
	
	jive_output * sum = jive_bitsum(2, (jive_output *[]){arg1, arg2});
	
	jive_subroutine_value_return(subroutine, 0, sum);
	
	jive_node_reserve(&subroutine->subroutine_node->base);
	jive_view(graph, stdout);
	
	jive_regselector regselector;
	jive_regselector_init(&regselector, graph, &jive_i386_reg_classifier);
	jive_regselector_process(&regselector);
	jive_i386_match_instructions(graph, &regselector);
	jive_regselector_fini(&regselector);
	
	jive_graph_prune(graph);
	jive_view(graph, stdout);
	
	jive_shaped_graph * shaped_graph = jive_regalloc(graph, &i386_xfer_factory);
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_view(graph, stdout);
	
	jive_buffer buffer;
	jive_buffer_init(&buffer, ctx);
	jive_graph_generate_code(graph, &buffer, &jive_i386_instructions[jive_i386_jump]);
	int (*function)(int, int) = (int(*)(int, int)) jive_buffer_executable(&buffer);
	jive_buffer_fini(&buffer);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	
	jive_context_destroy(ctx);
	
	int ret_value = function(18, 24);
	assert(ret_value == 42);
	
	return 0;
}

