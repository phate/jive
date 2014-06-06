/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/address.h>
#include <jive/arch/memlayout-simple.h>
#include <jive/context.h>
#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>

#include "testnodes.h"

static int test_main(void)
{
	setlocale(LC_ALL, "");
	
	jive_context * context = jive_context_create();
	
	jive_graph * graph = jive_graph_create(context);
	
	jive::bits::type bits32(32);
	const jive::value::type * tmparray0[] = {
		(jive::value::type *)&bits32,
		(jive::value::type *)&bits32
	};
	
	jive::rcd::declaration rec = {
		nelements : 2,
		elements : tmparray0
	};
	
	jive::addr::type addrtype;
	const jive::base::type * tmparray1[] = {&addrtype, &addrtype};
	
	jive_node * top = jive_test_node_create(graph->root_region,
		0, NULL, NULL,
		2, tmparray1);
	
	jive_output * memb1 = jive_memberof(top->outputs[0], &rec, 0);
	jive_output * memb2 = jive_memberof(top->outputs[0], &rec, 1);
	
	jive_output * cont1 = jive_containerof(memb1, &rec, 0);
	jive_output * cont2 = jive_containerof(memb2, &rec, 0);
	
	jive_output * cont3 = jive_containerof(top->outputs[1], &rec, 0);
	
	jive_output * memb3 = jive_memberof(cont3, &rec, 0);
	jive_output * memb4 = jive_memberof(cont3, &rec, 1);
	
	jive_view(graph, stdout);
	
	assert(cont1 == top->outputs[0]);
	assert(cont2 != top->outputs[0]);
	
	assert(memb4 != top->outputs[1]);
	assert(memb3 == top->outputs[1]);
	
	jive_output * zero = jive_bitconstant(graph, 32, "00000000000000000000000000000000");
	jive_output * one = jive_bitconstant(graph, 32, "10000000000000000000000000000000");
	jive_output * minus_one = jive_bitconstant(graph, 32, "11111111111111111111111111111111");
	
	jive_output * a0 = jive_arraysubscript(top->outputs[0], &bits32, zero);
	assert(a0 == top->outputs[0]);
	jive_output * a1 = jive_arraysubscript(top->outputs[0], &bits32, one);
	assert(a1 != top->outputs[0]);
	jive_output * tmp = jive_arraysubscript(a1, &bits32, minus_one);
	jive_view(graph, stdout);
	assert(tmp == a0);
	
	jive_output * diff = jive_arrayindex(a1, a0, &bits32, &bits32);
	assert(diff == one);
	
	jive_output * diff2 = jive_arrayindex(top->outputs[0], top->outputs[1], &bits32, &bits32);

	jive_memlayout_mapper_simple mapper;
	jive_memlayout_mapper_simple_init(&mapper, context, 32);

	jive_output * memberof = jive_memberof(cont3, &rec, 1);
	jive_output * arraysub = jive_arraysubscript(top->outputs[0],
		&bits32, one);

	const jive::base::type * tmparray2[] = {&addrtype, &addrtype, &bits32};
	jive_output * tmparray3[] = {memberof, arraysub, diff2};

	const jive::base::type * typeptr = &addrtype;

	jive_node * bottom = jive_test_node_create(graph->root_region,
		3, tmparray2,
			tmparray3,
		1, &typeptr);
	jive_graph_export(graph, bottom->outputs[0]);

	jive_containerof_node_address_transform(
		jive_containerof_node_cast(cont3->node()),
		&mapper.base.base);
	jive_memberof_node_address_transform(
		jive_memberof_node_cast(memberof->node()),
		&mapper.base.base);
	jive_arrayindex_node_address_transform(
		jive_arrayindex_node_cast(diff2->node()),
		&mapper.base.base);
	jive_arraysubscript_node_address_transform(
		jive_arraysubscript_node_cast(arraysub->node()),
		&mapper.base.base);
	
	jive_graph_prune(graph);
	jive_view(graph, stdout);
	
	jive_memlayout_mapper_simple_fini(&mapper);
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-address", test_main);
