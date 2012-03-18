#include <assert.h>
#include <stdio.h>
#include <locale.h>

#include <jive/arch/transfer-instructions.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/machine.h>
#include <jive/backend/i386/registerset.h>
#include <jive/backend/i386/subroutine.h>
#include <jive/regalloc.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/util/buffer.h>
#include <jive/vsdg.h>
#include <jive/view.h>

int main()
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_subroutine * subroutine = jive_i386_subroutine_create(
		graph->root_region,
		2, (jive_argument_type []) { jive_argument_int, jive_argument_int },
		1, (jive_argument_type []) { jive_argument_int });
	
	jive_region * fn_region = subroutine->region;
	
	jive_output * p1 = jive_subroutine_value_parameter(subroutine, 0);
	jive_output * p2 = jive_subroutine_value_parameter(subroutine, 1);
	
	jive_node * cmp = jive_instruction_node_create(
		fn_region, &jive_i386_instructions[jive_i386_int_cmp],
		(jive_output *[]){p1, p2}, NULL);
	
	jive_node * bge = jive_instruction_node_create(
		fn_region, &jive_i386_instructions[jive_i386_int_jump_sgreatereq],
		(jive_output *[]){cmp->outputs[0]}, (int64_t[]){0});
	
	const jive_type * bits32 = jive_output_get_type(p1);
	
	jive_output * max = *jive_choose(bge->outputs[0], 1,
		(const jive_type *[]){bits32},
		(jive_output *[]){p1}, (jive_output *[]){p2});
	
	jive_subroutine_value_return(subroutine, 0, max);
	
	jive_view(graph, stdout);
	
	jive_shaped_graph * shaped_graph = jive_regalloc(graph, &jive_i386_xfer_factory);
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_view(graph, stdout);
	
	jive_buffer buffer;
	jive_buffer_init(&buffer, ctx);
	jive_graph_generate_assembler(graph, &buffer, &jive_i386_instructions[jive_i386_jump]);
	fwrite(buffer.data, buffer.size, 1, stderr);
	jive_buffer_fini(&buffer);
	
	jive_buffer_init(&buffer, ctx);
	jive_graph_generate_code(graph, &buffer, &jive_i386_instructions[jive_i386_jump]);
	int (*function)(int, int) = (int(*)(int, int)) jive_buffer_executable(&buffer);
	jive_buffer_fini(&buffer);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	
	jive_context_destroy(ctx);
	
	int x, y;
	for (x = -10; x <= 10; ++x) {
		for (y = - 10; y <= 10 ; ++y) {
			int ret_value = function(x, y);
			assert(ret_value == ((x >= y) ? y : x));
		}
	}
	int ret_value = function(18, 24);
	assert(ret_value == 18);
	
	return 0;
}

