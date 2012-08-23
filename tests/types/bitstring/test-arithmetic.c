#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/vsdg.h>
#include <jive/view.h>
#include <jive/types/bitstring.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	jive_output * c0 = jive_bitconstant(graph, 4, "1100");
	jive_output * c1 = jive_bitconstant(graph, 4, "0001");

	jive_bitdifference(c0, c1);
	jive_bitshiproduct(c0, c1);
	jive_bituhiproduct(c0, c1);
	jive_bituquotient(c0, c1);
	jive_bitsquotient(c0, c1);
	jive_bitumod(c0, c1);
	jive_bitsmod(c0, c1);
	jive_bitshl(c0, c1);
	jive_bitshr(c0, c1);
	jive_bitashr(c0, c1);

	jive_view(graph, stdout);

	jive_graph_destroy(graph);

	assert(jive_context_is_empty(context));
	jive_context_destroy(context);		

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/test-arithmetic", test_main);
