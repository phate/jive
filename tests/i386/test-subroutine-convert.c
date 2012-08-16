/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <locale.h>
#include <assert.h>

#include <jive/vsdg.h>
#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/arch/address.h>
#include <jive/arch/addresstype.h>
#include <jive/backend/i386/subroutine.h>
#include <jive/types/function/fctlambda.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");
	
	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	jive_region * function = jive_function_region_create(graph->root_region);

	jive_node * top = jive_region_get_top_node(function);
	jive_node * bottom = jive_region_get_bottom_node(function);

	JIVE_DECLARE_ADDRESS_TYPE(addr);
	JIVE_DECLARE_BITSTRING_TYPE(bits32, 32);
	jive_gate * arg_gate = jive_type_create_gate(addr, graph, "arg2");
	jive_gate * ret_gate = jive_type_create_gate(addr, graph, "ret2");

	jive_output * arg = jive_node_gate_output(top, arg_gate);
	jive_output * constant = jive_bitconstant_unsigned(graph, 32, 2);
	jive_output * address = jive_arraysubscript(arg, jive_value_type_cast(bits32), constant);
	jive_node_gate_input(bottom, ret_gate, address);

	jive_node * lambda_node = jive_lambda_node_create(function);
	jive_node_reserve(lambda_node);

	jive_i386_subroutine_convert(graph->root_region, lambda_node);

	jive_view(graph, stdout);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("i386/test-subroutine-convert", test_main);
