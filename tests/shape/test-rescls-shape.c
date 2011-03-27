#include <assert.h>
#include <locale.h>
#include <jive/context.h>
#include <jive/arch/registers.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/vsdg/resource-private.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg.h>

const jive_register_class 
	gpr = {
		.base = {
			.name = "gpr",
			.limit = 4,
			.names = NULL,
			.parent = &jive_root_resource_class,
			.depth = 1,
		},
		.regs = NULL
	},
	evenreg = {
		.base = {
			.name = "even",
			.limit = 2,
			.names = NULL,
			.parent = &gpr.base,
			.depth = 2,
		},
		.regs = NULL
	},
	oddreg = {
		.base = {
			.name = "odd",
			.limit = 2,
			.names = NULL,
			.parent = &gpr.base,
			.depth = 2,
		},
		.regs = NULL
	},
	reg0 = {
		.base = {
			.name = "reg0",
			.limit = 1,
			.names = NULL,
			.parent = &evenreg.base,
			.depth = 3,
		},
		.regs = NULL
	},
	reg1 = {
		.base = {
			.name = "reg1",
			.limit = 1,
			.names = NULL,
			.parent = &oddreg.base,
			.depth = 3,
		},
		.regs = NULL
	},
	reg2 = {
		.base = {
			.name = "reg2",
			.limit = 1,
			.names = NULL,
			.parent = &evenreg.base,
			.depth = 3,
		},
		.regs = NULL
	},
	reg3 = {
		.base = {
			.name = "reg3",
			.limit = 1,
			.names = NULL,
			.parent = &oddreg.base,
			.depth = 3,
		},
		.regs = NULL
	}
;

int main()
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
	
	jive_variable_set_resource_class(r1, &reg0.base);
	jive_variable_set_resource_class(r2, &reg1.base);
	
	jive_shaped_graph * shaped_graph = jive_shaped_graph_create(graph);
	
	jive_shaped_region * root = jive_shaped_graph_map_region(shaped_graph, graph->root_region);
	jive_cut_append(jive_shaped_region_create_cut(root), bottom);
	jive_cut_append(jive_shaped_region_create_cut(root), mid);
	jive_cut_append(jive_shaped_region_create_cut(root), top);
	
	assert(jive_shaped_variable_can_merge(jive_shaped_graph_map_variable(shaped_graph, r2), r3));
	assert(!jive_shaped_variable_can_merge(jive_shaped_graph_map_variable(shaped_graph, r2), r1));
	jive_variable_set_resource_class(r3, &reg1.base);
	
	assert(jive_shaped_variable_check_change_resource_class(jive_shaped_graph_map_variable(shaped_graph, r2), &reg2.base) == 0);
	assert(jive_shaped_variable_check_change_resource_class(jive_shaped_graph_map_variable(shaped_graph, r2), &reg0.base) == &reg0.base);
	
	jive_resource_class_count * use_count = &jive_shaped_graph_map_node(shaped_graph, top)->use_count_after;
	const jive_resource_class * overflow = jive_resource_class_count_check_add(use_count, &reg0.base);
	assert(overflow == &reg0.base);
	
	(void) bottom;
	
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}
