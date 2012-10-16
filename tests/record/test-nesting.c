/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <stdio.h>

#include <jive/view.h>
#include <jive/context.h>
#include <jive/vsdg/graph.h>
#include <jive/types/record/rcdgroup.h>
#include <jive/types/record/rcdselect.h>
#include <jive/types/bitstring.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");
	
	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);
	
	static const jive_bitstring_type bits8 = {{{&JIVE_BITSTRING_TYPE}}, 8};
	static const jive_bitstring_type bits16 = {{{&JIVE_BITSTRING_TYPE}}, 16};
	static const jive_bitstring_type bits32 = {{{&JIVE_BITSTRING_TYPE}}, 32};
	
	jive_output * c0 = jive_bitconstant(graph, 8, "00000000");
	jive_output * c1 = jive_bitconstant(graph, 8, "00000001");
	jive_output * c2 = jive_bitconstant(graph, 16, "00001000000000000");
	jive_output * c3 = jive_bitconstant(graph, 32, "00000100000000000000000000000000");
	
	static const jive_value_type * l0_elements[] = { &bits8.base, &bits16.base };
	static const jive_record_declaration l0 = {2, l0_elements};
	
	jive_output * g0 = jive_group_create(&l0, 2, (jive_output * []){c1, c2});
	
	static const jive_record_type l0_type= {{{&JIVE_RECORD_TYPE}}, &l0};
	static const jive_value_type * l1_elements[] = { &bits8.base, &l0_type.base, &bits32.base};
	static const jive_record_declaration l1 = {3, l1_elements};

	jive_output * g1 = jive_group_create(&l1, 3, (jive_output * []){c0, g0, c3});
	
	jive_output * s0 = jive_select_create(2, g1);
	jive_output * s1 = jive_select_create(1, g1);
	jive_output * s2 = jive_select_create(1, s1);

	assert(jive_type_equals(jive_output_get_type(s0), jive_output_get_type(c3)));
	assert(jive_type_equals(jive_output_get_type(s1), jive_output_get_type(g0)));
	assert(jive_type_equals(jive_output_get_type(s2), jive_output_get_type(c2)));

	jive_view(graph, stderr);

	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);	

	return 0;
}

JIVE_UNIT_TEST_REGISTER("record/test-nesting", test_main);
