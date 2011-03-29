#include <assert.h>
#include <locale.h>
#include <jive/context.h>
#include <jive/arch/registers.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/vsdg/resource-private.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg.h>

extern const jive_register_class reg0, reg1, reg2, reg3;

const jive_register_name regs[] = {
	{
		.base = {
			.name = "r0",
			.resource_class = &reg0.base
		},
		.code = 0
	},
	{
		.base = {
			.name = "r1",
			.resource_class = &reg1.base
		},
		.code = 1
	},
	{
		.base = {
			.name = "r2",
			.resource_class = &reg2.base
		},
		.code = 2
	},
	{
		.base = {
			.name = "r3",
			.resource_class = &reg3.base
		},
		.code = 3
	}
};

const jive_resource_name * allnames [] = {&regs[0].base, &regs[2].base, &regs[1].base, &regs[3].base};

const jive_register_class 
	gpr = {
		.base = {
			.name = "gpr",
			.limit = 4,
			.names = allnames,
			.parent = &jive_root_resource_class,
			.depth = 1,
		},
		.regs = regs
	},
	evenreg = {
		.base = {
			.name = "even",
			.limit = 2,
			.names = allnames + 0,
			.parent = &gpr.base,
			.depth = 2,
		},
		.regs = regs
	},
	oddreg = {
		.base = {
			.name = "odd",
			.limit = 2,
			.names = allnames + 2,
			.parent = &gpr.base,
			.depth = 2,
		},
		.regs = regs + 2
	},
	reg0 = {
		.base = {
			.name = "reg0",
			.limit = 1,
			.names = allnames,
			.parent = &evenreg.base,
			.depth = 3,
		},
		.regs = regs
	},
	reg1 = {
		.base = {
			.name = "reg1",
			.limit = 1,
			.names = allnames + 2,
			.parent = &oddreg.base,
			.depth = 3,
		},
		.regs = regs + 2
	},
	reg2 = {
		.base = {
			.name = "reg2",
			.limit = 1,
			.names = allnames + 1,
			.parent = &evenreg.base,
			.depth = 3,
		},
		.regs = regs + 1
	},
	reg3 = {
		.base = {
			.name = "reg3",
			.limit = 1,
			.names = allnames + 3,
			.parent = &oddreg.base,
			.depth = 3,
		},
		.regs = regs + 3
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
	jive_variable_set_resource_class(r3, &gpr.base);
	
	jive_shaped_graph * shaped_graph = jive_shaped_graph_create(graph);
	
	jive_shaped_region * root = jive_shaped_graph_map_region(shaped_graph, graph->root_region);
	jive_cut_append(jive_shaped_region_create_cut(root), bottom);
	jive_cut_append(jive_shaped_region_create_cut(root), mid);
	jive_cut_append(jive_shaped_region_create_cut(root), top);
	
	assert(jive_shaped_variable_can_merge(jive_shaped_graph_map_variable(shaped_graph, r2), r3));
	assert(!jive_shaped_variable_can_merge(jive_shaped_graph_map_variable(shaped_graph, r2), r1));
	
	assert(jive_shaped_variable_check_change_resource_class(jive_shaped_graph_map_variable(shaped_graph, r2), &reg2.base) == 0);
	assert(jive_shaped_variable_check_change_resource_class(jive_shaped_graph_map_variable(shaped_graph, r2), &reg0.base) == &reg0.base);
	
	jive_resource_class_count * use_count = &jive_shaped_graph_map_node(shaped_graph, top)->use_count_after;
	const jive_resource_class * overflow = jive_resource_class_count_check_add(use_count, &reg0.base);
	assert(overflow == &reg0.base);
	
	assert(jive_shaped_graph_map_variable(shaped_graph, r1)->squeeze == 1);
	assert(jive_shaped_variable_allowed_resource_name_count(jive_shaped_graph_map_variable(shaped_graph, r1)) == 1);
	assert(jive_shaped_variable_allowed_resource_name(jive_shaped_graph_map_variable(shaped_graph, r1), allnames[0]));
	
	assert(jive_shaped_graph_map_variable(shaped_graph, r3)->squeeze == 1);
	assert(jive_shaped_variable_allowed_resource_name_count(jive_shaped_graph_map_variable(shaped_graph, r3)) == 4);
	
	assert(jive_shaped_variable_interferes_with(jive_shaped_graph_map_variable(shaped_graph, r3), jive_shaped_graph_map_variable(shaped_graph, r1)));
	jive_variable_set_resource_name(r1, allnames[0]);
	
	assert(jive_shaped_graph_map_variable(shaped_graph, r1)->squeeze == 0);
	assert(jive_shaped_graph_map_variable(shaped_graph, r3)->squeeze == 0);
	assert(jive_shaped_variable_allowed_resource_name_count(jive_shaped_graph_map_variable(shaped_graph, r3)) == 3);
	assert(!jive_shaped_variable_allowed_resource_name(jive_shaped_graph_map_variable(shaped_graph, r3), allnames[0]));
	
	(void) bottom;
	
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}
