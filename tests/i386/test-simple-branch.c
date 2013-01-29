/*
 * Copyright 2010 2011 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <stdio.h>

#include <jive/arch/codegen.h>
#include <jive/arch/label-mapper.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/machine.h>
#include <jive/backend/i386/registerset.h>
#include <jive/backend/i386/relocation.h>
#include <jive/backend/i386/subroutine.h>
#include <jive/regalloc.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/view.h>
#include <jive/vsdg.h>

static int test_main(void)
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
	
	jive_output * max;
	jive_gamma(bge->outputs[0], 1,
		(const jive_type *[]){bits32},
		(jive_output *[]){p2}, (jive_output *[]){p1}, &max);
	
	jive_subroutine_value_return(subroutine, 0, max);
	
	jive_view(graph, stdout);
	
	jive_shaped_graph * shaped_graph = jive_regalloc(graph);
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_view(graph, stdout);
	
	jive_buffer buffer;
	jive_buffer_init(&buffer, ctx);
	jive_label_name_mapper * name_mapper = jive_label_name_mapper_simple_create(ctx, NULL, 0);
	jive_graph_generate_assembler(graph, name_mapper, &buffer);
	jive_label_name_mapper_destroy(name_mapper);
	fwrite(buffer.data, buffer.size, 1, stderr);
	jive_buffer_fini(&buffer);

	jive_compilate compilate;
	jive_compilate_init(&compilate, ctx);
	jive_label_symbol_mapper * symbol_mapper = jive_label_symbol_mapper_simple_create(ctx);
	jive_graph_generate_code(graph, symbol_mapper, &compilate);
	jive_label_symbol_mapper_destroy(symbol_mapper);
	jive_compilate_map * map = jive_compilate_load(&compilate,
		NULL, jive_i386_process_relocation);
	void * result = jive_compilate_map_get_stdsection(map, jive_stdsectionid_code);
	int (*function)(int, int) = (int(*)(int, int)) result;
	jive_compilate_map_destroy(map);
	jive_compilate_fini(&compilate);
	
	jive_graph_destroy(graph);
	jive_context_assert_clean(ctx);
	
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


JIVE_UNIT_TEST_REGISTER("i386/test-simple-branch", test_main);
