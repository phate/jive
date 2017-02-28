/*
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>

#include <jive/arch/codegen.h>
#include <jive/arch/dataobject.h>
#include <jive/arch/instruction.h>
#include <jive/arch/label-mapper.h>
#include <jive/arch/memlayout-simple.h>
#include <jive/types/bitstring/constant.h>
#include <jive/view.h>
#include <jive/vsdg.h>

#include "testnodes.h"

static int test_main(void)
{
	jive::graph * &graph = jive_graph_create();

	jive::output * c8 = jive_bitconstant_unsigned(graph.root(), 8, 8);
	jive::output * c16 = jive_bitconstant_unsigned(graph.root(), 16, 16);
	jive::output * c32 = jive_bitconstant_unsigned(graph.root(), 32, 32);

	jive::memlayout_mapper_simple mapper(4);
	jive_dataobj(c8, &mapper);
	jive_rodataobj(c16, &mapper);
	jive_bssobj(c32, &mapper);

	jive::view(graph->root(), stderr);

	jive_compilate compilate;
	jive_compilate_init(&compilate);
	jive_label_symbol_mapper * symbol_mapper = jive_label_symbol_mapper_simple_create();
	jive_graph_generate_code(&graph, symbol_mapper, &compilate);
	jive_label_symbol_mapper_destroy(symbol_mapper);

	jive_buffer * data_buffer = jive_compilate_get_buffer(&compilate,
		jive_stdsectionid_data);
	assert(data_buffer->data.size() == 1);
	assert(data_buffer->data[0] == 8);

	jive_buffer * rodata_buffer = jive_compilate_get_buffer(&compilate,
		jive_stdsectionid_rodata);
	assert(rodata_buffer->data.size() == 2);
	assert(rodata_buffer->data[0] == 16);

	jive_buffer * bss_buffer = jive_compilate_get_buffer(&compilate,
		jive_stdsectionid_bss);
	assert(bss_buffer->data.size() == 4);
	assert(bss_buffer->data[0] == 32);
	
	jive_compilate_fini(&compilate);
	jive_graph_destroy(&graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-codegen", test_main);
