/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <stdio.h>
#include <locale.h>

#include <jive/vsdg.h>
#include <jive/view.h>
#include <jive/arch/codegen_buffer.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/machine.h>
#include <jive/backend/i386/registerset.h>
#include <jive/backend/i386/subroutine.h>

#include <jive/regalloc.h>
#include <jive/regalloc/shaped-graph.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_subroutine * subroutine = jive_i386_subroutine_create(graph->root_region,
		2, (jive_argument_type []) { jive_argument_int, jive_argument_int },
		1, (jive_argument_type []) { jive_argument_int });
	
	jive_output * arg1 = jive_subroutine_value_parameter(subroutine, 0);
	jive_output * arg2 = jive_subroutine_value_parameter(subroutine, 1);
	
	jive_node * add = jive_instruction_node_create(
		subroutine->region,
		&jive_i386_instructions[jive_i386_int_add],
		(jive_output *[]){arg1, arg2}, NULL);
	
	jive_subroutine_value_return(subroutine, 0, add->outputs[0]);
	
	jive_view(graph, stdout);
	
	jive_shaped_graph * shaped_graph = jive_regalloc(graph);
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_view(graph, stdout);
	
	jive_codegen_buffer buffer;
	jive_codegen_buffer_init(&buffer, ctx);
	jive_graph_generate_code(graph, &buffer);
	int (*function)(int, int) = (int(*)(int, int)) jive_codegen_buffer_map_to_memory(&buffer);
	jive_codegen_buffer_fini(&buffer);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	
	jive_context_destroy(ctx);
	
	int ret_value = function(18, 24);
	assert(ret_value == 42);
	
	return 0;
}


JIVE_UNIT_TEST_REGISTER("i386/simple-regalloc", test_main);
