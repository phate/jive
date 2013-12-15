/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <jive/context.h>
#include <jive/arch/registers.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/vsdg/resource-private.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg.h>

#include "testarch.h"

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_region * region = graph->root_region;
	
	JIVE_DECLARE_TYPE(type);
	
	jive_node * top = jive_node_create(region,
		0, NULL, NULL,
		2, (const jive_type *[]){type, type});
	
	jive_node * mid = jive_node_create(region,
		1, (const jive_type *[]){type, }, (jive_output *[]){top->outputs[0]},
		1, (const jive_type *[]){type});
	
	jive_node * bottom = jive_node_create(region,
		2, (const jive_type *[]){type, type}, (jive_output *[]){mid->outputs[0], top->outputs[1]},
		0, NULL);
	
	jive_variable * r1 = jive_output_auto_merge_variable(top->outputs[1])->variable;
	jive_variable * r2 = jive_output_auto_merge_variable(top->outputs[0])->variable;
	jive_variable * r3 = jive_output_auto_merge_variable(mid->outputs[0])->variable;
	
	jive_variable_set_resource_class(r1, &jive_testarch_regcls_r0.base);
	jive_variable_set_resource_class(r2, &jive_testarch_regcls_r1.base);
	jive_variable_set_resource_class(r3, &jive_testarch_regcls_gpr.base);
	
	jive_shaped_graph * shaped_graph = jive_shaped_graph_create(graph);
	
	jive_shaped_region * root = jive_shaped_graph_map_region(shaped_graph, graph->root_region);
	jive_cut_append(jive_shaped_region_create_cut(root), bottom);
	jive_cut_append(jive_shaped_region_create_cut(root), mid);
	jive_cut_append(jive_shaped_region_create_cut(root), top);
	
	assert(jive_shaped_variable_can_merge(jive_shaped_graph_map_variable(shaped_graph, r2), r3));
	assert(!jive_shaped_variable_can_merge(jive_shaped_graph_map_variable(shaped_graph, r2), r1));
	
	assert(jive_shaped_variable_check_change_resource_class(jive_shaped_graph_map_variable(shaped_graph, r2), &jive_testarch_regcls_r2.base) == 0);
	assert(jive_shaped_variable_check_change_resource_class(jive_shaped_graph_map_variable(shaped_graph, r2), &jive_testarch_regcls_r0.base) == &jive_testarch_regcls_r0.base);
	
	jive_resource_class_count * use_count = &jive_shaped_graph_map_node(shaped_graph, top)->use_count_after;
	const jive_resource_class * overflow = jive_resource_class_count_check_add(use_count, &jive_testarch_regcls_r0.base);
	assert(overflow == &jive_testarch_regcls_r0.base);
	
	assert(jive_shaped_graph_map_variable(shaped_graph, r1)->squeeze == 1);
	assert(jive_shaped_variable_allowed_resource_name_count(jive_shaped_graph_map_variable(shaped_graph, r1)) == 1);
	assert(jive_shaped_variable_allowed_resource_name(jive_shaped_graph_map_variable(shaped_graph, r1), &jive_testarch_reg_r0.base));
	
	assert(jive_shaped_graph_map_variable(shaped_graph, r3)->squeeze == 1);
	assert(jive_shaped_variable_allowed_resource_name_count(jive_shaped_graph_map_variable(shaped_graph, r3)) == 4);
	
	assert(jive_shaped_variable_interferes_with(jive_shaped_graph_map_variable(shaped_graph, r3), jive_shaped_graph_map_variable(shaped_graph, r1)));
	
	assert(shaped_graph->var_assignment_tracker.trivial.first == jive_shaped_graph_map_variable(shaped_graph, r2) || shaped_graph->var_assignment_tracker.trivial.last == jive_shaped_graph_map_variable(shaped_graph, r2));
	assert(shaped_graph->var_assignment_tracker.trivial.first == jive_shaped_graph_map_variable(shaped_graph, r3) || shaped_graph->var_assignment_tracker.trivial.last == jive_shaped_graph_map_variable(shaped_graph, r3));
	assert(shaped_graph->var_assignment_tracker.pressured_max == 1);
	assert(shaped_graph->var_assignment_tracker.pressured[0].first == jive_shaped_graph_map_variable(shaped_graph, r1));
	
	jive_variable_set_resource_name(r1, &jive_testarch_reg_r0.base);
	
	assert(jive_shaped_graph_map_variable(shaped_graph, r1)->squeeze == 0);
	assert(jive_shaped_graph_map_variable(shaped_graph, r3)->squeeze == 0);
	assert(jive_shaped_variable_allowed_resource_name_count(jive_shaped_graph_map_variable(shaped_graph, r3)) == 3);
	assert(!jive_shaped_variable_allowed_resource_name(jive_shaped_graph_map_variable(shaped_graph, r3), &jive_testarch_reg_r0.base));
	
	assert(shaped_graph->var_assignment_tracker.assigned.first == jive_shaped_graph_map_variable(shaped_graph, r1));
	
	(void) bottom;
	
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("shape/test-rescls-shape", test_main);
