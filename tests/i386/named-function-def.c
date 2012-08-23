#include "test-registry.h"

#include <assert.h>
#include <stdio.h>
#include <locale.h>

#include <jive/vsdg.h>
#include <jive/view.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/objdef.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/registerset.h>
#include <jive/backend/i386/machine.h>
#include <jive/backend/i386/classifier.h>
#include <jive/backend/i386/instrmatch.h>
#include <jive/backend/i386/subroutine.h>
#include <jive/types/bitstring/arithmetic.h>

#include <jive/regalloc.h>
#include <jive/regalloc/shaped-graph.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_region * fn_region = jive_function_region_create(graph->root_region);
	JIVE_DECLARE_BITSTRING_TYPE(bits32, 32);
	
	jive_output * a = jive_node_add_output(fn_region->top, bits32);
	jive_output * b = jive_node_add_output(fn_region->top, bits32);
	jive_output * sum = jive_bitsum(2, (jive_output *[]){a, b});
	jive_node_add_input(fn_region->bottom, bits32, sum);
	
	jive_node * abstract_fn = jive_lambda_node_create(fn_region);
	
	jive_subroutine * i386_fn = jive_i386_subroutine_convert(graph->root_region, abstract_fn);
	jive_node * fn_name = jive_objdef_node_create(jive_subroutine_objdef(i386_fn), "add_int32");
	
	jive_node_reserve(fn_name);
	jive_graph_prune(graph);
	
	jive_view(graph, stdout);
	
	jive_regselector regselector;
	jive_regselector_init(&regselector, graph, &jive_i386_reg_classifier);
	jive_regselector_process(&regselector);
	jive_i386_match_instructions(graph, &regselector);
	jive_regselector_fini(&regselector);
	
	jive_graph_prune(graph);
	//jive_view(graph, stdout);
	
	jive_shaped_graph * shaped_graph = jive_regalloc(graph);
	jive_shaped_graph_destroy(shaped_graph);
	
	//jive_view(graph, stdout);
	
	jive_buffer buffer;
	jive_buffer_init(&buffer, ctx);
	jive_graph_generate_assembler(graph, &buffer);
	fwrite(buffer.data, buffer.size, 1, stdout);
	jive_buffer_fini(&buffer);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	
	jive_context_destroy(ctx);
	
	return 0;
}


JIVE_UNIT_TEST_REGISTER("i386/named-function-def", test_main);
