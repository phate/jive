/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <stdio.h>

#include <jive/arch/codegen.h>
#include <jive/arch/label-mapper.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/registerset.h>
#include <jive/backend/i386/relocation.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-variable.h>
#include <jive/view.h>
#include <jive/vsdg.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_graph * graph = jive_graph_create();
	const int64_t tmparray0[] = {42};

	graph->root_region->attrs.section = jive_region_section_code;
	jive_node * enter = jive_instruction_node_create(
		graph->root_region,
		&jive_i386_instr_int_load_imm,
		0, tmparray0);
	
	jive_node * leave = jive_instruction_node_create(graph->root_region, &jive_i386_instr_ret,
		{}, {}, {}, {}, {&jive::ctl::boolean});
	
	const jive::base::type * type = &enter->outputs[0]->type();
	jive::gate * gate = jive_graph_create_gate(graph, "retval", *type);
	gate -> required_rescls = &jive_i386_regcls_gpr_eax.base;
	jive_node_gate_input(leave, gate, enter->outputs[0]);
	
	jive_view(graph, stderr);
	
	jive_shaped_graph * shaped_graph = jive_shaped_graph_create(graph);
	
	jive_ssavar * ssavar = jive_output_auto_merge_variable(enter->outputs[0]);
	
	jive_shaped_variable * regvar = jive_shaped_graph_map_variable(shaped_graph, ssavar->variable);
	
	const jive_resource_name * reg_eax = &jive_i386_reg_eax.base;
	assert(jive_shaped_variable_allowed_resource_name(regvar, reg_eax));
	assert(jive_shaped_variable_allowed_resource_name_count(regvar) == 1);
	
	jive_variable_set_resource_name(ssavar->variable, reg_eax);
	
	jive_view(graph, stderr);
	
	jive_compilate compilate;
	jive_compilate_init(&compilate);
	jive_label_symbol_mapper * symbol_mapper = jive_label_symbol_mapper_simple_create();
	jive_graph_generate_code(graph, symbol_mapper, &compilate);
	jive_label_symbol_mapper_destroy(symbol_mapper);
	jive_compilate_map * map = jive_compilate_load(&compilate,
		NULL, jive_i386_process_relocation);
	void * result = jive_compilate_map_get_stdsection(map, jive_stdsectionid_code);
	int (*function)(void) = (int(*)(void)) result;
	jive_compilate_map_destroy(map);
	jive_compilate_fini(&compilate);
	
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_graph_destroy(graph);

	int ret_value = function();
	assert(ret_value == 42);
	
	return 0;
}


JIVE_UNIT_TEST_REGISTER("i386/simple-function", test_main);
