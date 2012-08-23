#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <stdio.h>

#include <jive/view.h>
#include <jive/context.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/union.h>
#include <jive/types/bitstring.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	static const jive_bitstring_type bits8 = {{{&JIVE_BITSTRING_TYPE}}, 8};
	static const jive_bitstring_type bits16 = {{{&JIVE_BITSTRING_TYPE}}, 16};
	static const jive_bitstring_type bits32 = {{{&JIVE_BITSTRING_TYPE}}, 32};
	
	jive_output * c2 = jive_bitconstant(graph, 16, "00001000000000000");
	
	static const jive_value_type * l0_elements[] = { &bits8.base, &bits16.base };
	static const jive_union_declaration l0 = {2, l0_elements};
	
	jive_output * u0 = jive_unify_create(&l0, 1, c2);

	static const jive_union_type l0_type= {{{&JIVE_UNION_TYPE}}, &l0};
	static const jive_value_type * l1_elements[] = { &bits8.base, &l0_type.base, &bits32.base};
	static const jive_union_declaration l1 = {3, l1_elements};

	jive_output * u1 = jive_unify_create(&l1, 1, u0);

	jive_output * s1 = jive_choose_create(1, u1);
	jive_output * s2 = jive_choose_create(1, s1);

	assert(jive_type_equals(jive_output_get_type(s1), jive_output_get_type(u0)));
	assert(jive_type_equals(jive_output_get_type(s2), jive_output_get_type(c2)));

	jive_view(graph, stderr);

	jive_graph_destroy(graph);

	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("union/test-nesting", test_main);
