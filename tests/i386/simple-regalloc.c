/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <stdio.h>
#include <locale.h>

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
	jive_argument_type  tmparray0[] = { jive_argument_int, jive_argument_int };
	jive_argument_type  tmparray1[] = { jive_argument_int };
	
	jive_subroutine subroutine = jive_i386_subroutine_begin(graph,
		2, tmparray0,
		1, tmparray1);
	
	jive::output * arg1 = jive_subroutine_simple_get_argument(subroutine, 0);
	jive::output * arg2 = jive_subroutine_simple_get_argument(subroutine, 1);
	jive::output * tmparray2[] = {arg1, arg2};
	
	jive_node * add = jive_instruction_node_create(
		subroutine.region,
		&jive_i386_instr_int_add,
		tmparray2, NULL);
	
	jive_subroutine_simple_set_result(subroutine, 0, add->outputs[0]);
	
	jive_graph_export(graph, jive_subroutine_end(subroutine)->outputs[0]);
	
	jive_view(graph, stdout);
	
	jive_shaped_graph * shaped_graph = jive_regalloc(graph);
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_view(graph, stdout);
	
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
	assert(jive_context_is_empty(ctx));
	
	jive_context_destroy(ctx);
	
	int ret_value = function(18, 24);
	assert(ret_value == 42);
	
	return 0;
}


JIVE_UNIT_TEST_REGISTER("i386/simple-regalloc", test_main);
