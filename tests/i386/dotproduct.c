/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <stdio.h>

#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/machine.h>
#include <jive/backend/i386/registerset.h>
#include <jive/backend/i386/stackframe.h>
#include <jive/backend/i386/subroutine.h>
#include <jive/util/buffer.h>
#include <jive/view.h>
#include <jive/vsdg.h>

#include <jive/regalloc.h>

typedef long (*dotprod_function_t)(const int *, const int *);

dotprod_function_t
make_dotprod_function(size_t vector_size)
{
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	jive_argument_type  tmparray0[] = { jive_argument_pointer, jive_argument_pointer };
	
	jive_subroutine subroutine = jive_i386_subroutine_begin(
		graph,
		2, tmparray0,
		jive_argument_int);
	
	jive::output * p1 = jive_subroutine_simple_get_argument(subroutine, 0);
	jive::output * p2 = jive_subroutine_simple_get_argument(subroutine, 1);
	
	jive::output * operands[vector_size];
	size_t n;
	for(n=0; n<vector_size; n++) {
		long displacement = n * 4;
		jive_node * a1 = (jive_node *) jive_instruction_node_create(
			subroutine->region,
			&jive_i386_instr_int_load32_disp,
			&p1, &displacement);
		jive::output * v1 = a1->outputs[0];
		jive_node * a2 = (jive_node *) jive_instruction_node_create(
			subroutine.region,
			&jive_i386_instr_int_load32_disp,
			&p2, &displacement);
		jive::output * v2 = a2->outputs[0];
		jive::output * tmparray1[] = {v1, v2};
		
		jive_node * m = (jive_node *) jive_instruction_node_create(
			subroutine.region,
			&jive_i386_instr_int_mul,
			tmparray1, 0);
		operands[n] = m->outputs[0];
	}
	
	jive::output * value = operands[0];
	for(n=1; n<vector_size; n++) {
		jive::output * tmparray2[] = {value, operands[n]};
		jive_node * s = (jive_node *) jive_instruction_node_create(
			subroutine.region,
			&jive_i386_instr_int_add,
			tmparray2, 0);
		value = s->outputs[0];
	}
	
	jive_subroutine_simple_set_result(subroutine, 0, value);
	jive_graph_export(graph, jive_subroutine_end(subroutine)->outputs[0]);
	
	jive_regalloc(graph);
	
	jive_buffer buffer;
	jive_buffer_init(&buffer, ctx);
	jive_graph_generate_code(graph, &buffer);
	dotprod_function_t function = (dotprod_function_t) jive_buffer_executable(&buffer);
	
	jive_buffer_fini(&buffer);
	
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	
	return function;
}


static int test_main(void)
{
	setlocale(LC_ALL, "");
	
	size_t count = 4, n;
	if (argc>=2) count = atoi(argv[1]);
	if (count<4) count = 4;
	dotprod_function_t dotprod2 = make_dotprod_function(count);
	
	int a[count], b[count];
	for(n=0; n<count; n++) a[n] = b[n] = 0;
	a[0] = 1; a[1] = 4; a[2] = 0; a[3] = -3;
	b[0] = 3; b[1] = 7; b[2] = 0; b[3] = 5;
	int result = dotprod2(a, b);
	assert(result == 1*3 + 4*7 + -3*5 );
	
	return 0;
}


JIVE_UNIT_TEST_REGISTER("i386/dotproduct", test_main);
