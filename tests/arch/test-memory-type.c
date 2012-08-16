/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/vsdg.h>
#include <jive/arch/memory.h>
#include <jive/types/function/fctlambda.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	jive_region * function = jive_function_region_create(graph->root_region);
	
	jive_node * top = jive_region_get_top_node(function);
	jive_node * bottom = jive_region_get_bottom_node(function);

	JIVE_DECLARE_MEMORY_TYPE(memtype);
	jive_gate * arg_gate = jive_type_create_gate(memtype, graph, "arg");
	jive_gate * ret_gate = jive_type_create_gate(memtype, graph, "ret");

	jive_output * arg = jive_node_gate_output(top, arg_gate);
	jive_input * ret = jive_node_gate_input(bottom, ret_gate, arg);

	assert(jive_output_isinstance(arg, &JIVE_STATE_OUTPUT));
	assert(jive_input_isinstance(ret, &JIVE_STATE_INPUT));
	assert(jive_gate_isinstance(arg_gate, &JIVE_STATE_GATE));
	assert(jive_type_isinstance(memtype, &JIVE_STATE_TYPE));

	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);		

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-memory-type", test_main);
