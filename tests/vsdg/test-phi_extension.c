/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/phi.h>

static int
test_main()
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	JIVE_DECLARE_VALUE_TYPE(vtype);
	jive_phi phi = jive_phi_begin(graph);
	jive_phi_fixvar fv = jive_phi_fixvar_enter(phi, vtype);
	jive_phi_fixvar_leave(phi, fv.gate, fv.value);
	jive_phi_end(phi, 1, &fv);

	jive_view(graph, stderr);

	jive_phi_node * phi_node = jive_phi_node_cast(fv.value->node);
	jive_phi_extension * phi_ext = jive_phi_begin_extension(phi_node,
		2, (const jive_type *[]){vtype, vtype});
	jive_output ** results = jive_phi_end_extension(phi_ext);

	jive_view(graph, stderr);

	assert(phi_node->base.base.noutputs == 3);
	assert(phi_node->base.base.outputs[0] == fv.value);
	assert(phi_node->base.base.outputs[1] == results[0]);
	assert(phi_node->base.base.outputs[2] == results[1]);

	jive_graph_destroy(graph);
	jive_context_assert_clean(context);
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-phi_extension", test_main);
