/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <string.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/address.h>
#include <jive/arch/linker-symbol.h>
#include <jive/arch/memlayout-simple.h>
#include <jive/view.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/node-private.h>

#include "testnodes.h"

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();
	
	jive_label_external foobar, bla;
	
	jive_linker_symbol foobar_symbol;
	jive_linker_symbol bla_symbol;
	jive_label_external_init(&foobar, "foobar", &foobar_symbol);
	jive_label_external_init(&bla, "bla", &bla_symbol);

	jive::output * o0 = jive_label_to_address_create(graph, &foobar.base);
	jive::output * o1 = jive_label_to_address_create(graph, &bla.base);

	const jive::address::label_to_address_op * attrs0;
	const jive::address::label_to_address_op * attrs1;
	attrs0 = dynamic_cast<const jive::address::label_to_address_op*>(&o0->node()->operation());
	attrs1 = dynamic_cast<const jive::address::label_to_address_op*>(&o1->node()->operation());

	assert(attrs0->label() == &foobar.base);
	assert(attrs1->label() == &bla.base);
	
	assert(o0->node()->operation() != *attrs1);
	
	jive::output * o2 = jive_label_to_bitstring_create(graph, &foobar.base, 32);
	jive::output * o3 = jive_label_to_bitstring_create(graph, &bla.base, 32);
	jive::output * o4 = jive_label_to_bitstring_create(graph, &foobar.base, 16);

	const jive::address::label_to_bitstring_op * attrs2;
	const jive::address::label_to_bitstring_op * attrs3;
	const jive::address::label_to_bitstring_op * attrs4;
	attrs2 = dynamic_cast<const jive::address::label_to_bitstring_op*>(&o2->node()->operation());
	attrs3 = dynamic_cast<const jive::address::label_to_bitstring_op*>(&o3->node()->operation());
	attrs4 = dynamic_cast<const jive::address::label_to_bitstring_op*>(&o4->node()->operation());

	assert(attrs2->label() == &foobar.base);
	assert(attrs3->label() == &bla.base);
	assert(attrs4->label() == &foobar.base);
	
	assert(o2->node()->operation() != *attrs4);
	assert(o2->node()->operation() != *attrs3);
	
	jive::addr::type addr;
	jive::bits::type bits32(32);
	jive::bits::type bits16(16);
	jive_node * bottom = jive_test_node_create(graph->root_region,
		{&addr, &addr, &bits32, &bits32, &bits16}, {o0, o1, o2, o3, o4}, {&addr});
	jive_graph_export(graph, bottom->outputs[0]);

	jive_view(graph, stderr);

	jive_memlayout_mapper_simple mapper;
	jive_memlayout_mapper_simple_init(&mapper, 32);

	jive_node_address_transform(o0->node(), &mapper.base);
	jive_node_address_transform(o1->node(), &mapper.base);

	jive_memlayout_mapper_simple_fini(&mapper);

	jive_graph_prune(graph);
	jive_view(graph, stderr);

	jive_graph_destroy(graph);

	return 0;
}


JIVE_UNIT_TEST_REGISTER("arch/test-label-nodes", test_main);
