#include <assert.h>
#include <locale.h>
#include <jive/context.h>
#include <jive/arch/registers.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/resource-private.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg.h>
#include <jive/view.h>

static jive_shaped_node *
shape(jive_shaped_graph * shaped_graph, jive_node * node)
{
	size_t n;
	for(n = 0; n < node->ninputs; n ++) {
		jive_input * input = node->inputs[n];
		jive_input_auto_merge_variable(input);
		jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(shaped_graph, input->ssavar);
		jive_shaped_ssavar_lower_boundary_region_depth(shaped_ssavar, node->region->depth);
	}
	for(n = 0; n < node->noutputs; n ++) {
		jive_output * output = node->outputs[n];
		jive_output_auto_merge_variable(output);
		jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(shaped_graph, output->ssavar);
		jive_shaped_ssavar_lower_boundary_region_depth(shaped_ssavar, node->region->depth);
	}
	jive_shaped_region * shaped_region = jive_shaped_graph_map_region(shaped_graph, node->region);
	jive_cut * cut = jive_shaped_region_create_cut(shaped_region);
	return jive_cut_append(cut, node);
}

int main()
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_region * root = graph->root_region;
	
	JIVE_DECLARE_TYPE(type);
	JIVE_DECLARE_ANCHOR_TYPE(anchor_type);
	
	jive_node * top = jive_node_create(root,
		0, NULL, NULL,
		2, (const jive_type * []){type, type});
	
	jive_region * loop_region = jive_region_create_subregion(root);
	loop_region->is_looped = true;
	
	jive_node * loop_head = jive_node_create(loop_region,
		1, (const jive_type * []){type}, (jive_output *[]){top->outputs[0]},
		1, (const jive_type * []){type});
	loop_region->top = loop_head;
	
	jive_node * loop_body = jive_node_create(loop_region,
		2, (const jive_type * []){type, type}, (jive_output *[]){loop_head->outputs[0], top->outputs[1]},
		1, (const jive_type * []){type});
	
	jive_node * loop_tail = jive_node_create(loop_region,
		1, (const jive_type * []){type}, (jive_output *[]){loop_body->outputs[0]},
		1, (const jive_type * []){anchor_type});
	loop_region->bottom = loop_tail;
	
	jive_node * loop_anchor = jive_node_create(root,
		1, (const jive_type * []){anchor_type}, (jive_output *[]){loop_tail->outputs[0]},
		1, (const jive_type * []){type});
	
	jive_node * bottom = jive_node_create(root,
		1, (const jive_type * []){type}, (jive_output *[]){loop_anchor->outputs[0]},
		0, NULL);
	
	jive_node_reserve(bottom);
	
	jive_view(graph, stderr);
	
	jive_shaped_graph * shaped_graph = jive_shaped_graph_create(graph);
	
	jive_shaped_node * shaped_bottom = shape(shaped_graph, bottom);
	jive_shaped_node * shaped_anchor = shape(shaped_graph, loop_anchor);
	jive_shaped_node * shaped_tail = shape(shaped_graph, loop_tail);
	jive_shaped_node * shaped_body = shape(shaped_graph, loop_body);
	
	(void) shaped_bottom;
	(void) shaped_anchor;
	(void) shaped_tail;
	(void) shaped_body;
	
	assert(jive_shaped_ssavar_is_active_before(jive_shaped_graph_map_ssavar(shaped_graph, loop_body->inputs[1]->ssavar), shaped_tail));
	
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}
