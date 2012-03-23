#include <assert.h>
#include <stdio.h>
#include <locale.h>
#include <jive/vsdg.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/node-private.h>
#include <jive/view.h>

#include <jive/regalloc/reroute.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-region.h>
#include <jive/regalloc/shaped-node-private.h>
#include <jive/regalloc/shaped-variable-private.h>
#include <jive/types/bitstring.h>

static void
shape(jive_shaped_graph * shaped_graph, jive_node * node)
{
	size_t n;
	for (n = 0; n < node->noutputs; n++) {
		jive_ssavar * ssavar = jive_output_auto_merge_variable(node->outputs[n]);
		jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(shaped_graph, ssavar);
		jive_shaped_ssavar_lower_boundary_region_depth(shaped_ssavar, node->region->depth);
	}
	for (n = 0; n < node->ninputs; n++) {
		jive_ssavar * ssavar = jive_input_auto_merge_variable(node->inputs[n]);
		jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(shaped_graph, ssavar);
		jive_shaped_ssavar_lower_boundary_region_depth(shaped_ssavar, node->region->depth);
	}
	jive_shaped_region * shaped_region = jive_shaped_graph_map_region(shaped_graph, node->region);
	jive_cut * cut = jive_shaped_region_create_cut(shaped_region);
	jive_cut_append(cut, node);
}

int main()
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	JIVE_DECLARE_TYPE(type);
	JIVE_DECLARE_ANCHOR_TYPE(anchor_type);
	JIVE_DECLARE_CONTROL_TYPE(control_type);
	
	jive_node * n1 = jive_node_create(graph->root_region,
		0, NULL, NULL,
		1, (const jive_type *[]){type});
	
	jive_region * r1 = jive_region_create_subregion(graph->root_region);
	
	jive_node * n2 = jive_node_create(r1,
		1, (const jive_type *[]){type}, n1->outputs,
		4, (const jive_type *[]){type, type, type, control_type});
	
	jive_output * tmp;
	jive_gamma(n2->outputs[3],
		1, (const jive_type *[]){type},
		(jive_output *[]){n2->outputs[0]}, (jive_output *[]){n2->outputs[1]}, &tmp);
	jive_node * n5 = tmp->node;
	
	jive_node * n3 = n5->inputs[0]->origin->node;
	jive_node * n4 = n5->inputs[1]->origin->node;
	
	jive_node * xfer = jive_node_create(n4->region,
		1, (const jive_type *[]){type}, (jive_output *[]){n2->outputs[1]},
		1, (const jive_type *[]){type});
	jive_input_divert_origin(n4->inputs[0], xfer->outputs[0]);
	
	jive_node * n6 = jive_node_create(r1,
		2, (const jive_type *[]){type, type}, (jive_output *[]){n5->outputs[0], n2->outputs[2]},
		1, (const jive_type *[]){anchor_type});
	
	jive_node * n7 = jive_node_create(graph->root_region,
		1, (const jive_type *[]){anchor_type}, n6->outputs,
		0, NULL);
	
	jive_shaped_graph * shaped_graph = jive_shaped_graph_create(graph);
	
	shape(shaped_graph, n7);
	shape(shaped_graph, n6);
	shape(shaped_graph, n5);
	shape(shaped_graph, n4);
	shape(shaped_graph, n3);
	shape(shaped_graph, xfer);
	shape(shaped_graph, n2);
	shape(shaped_graph, n1);
	
	jive_view(graph, stdout);
	
	jive_regalloc_reroute_at_point(n2->outputs[2]->ssavar, jive_shaped_graph_map_node(shaped_graph, n4));
	
	assert(n3->ninputs == 2 && n4->ninputs == 2 && n5->noutputs == 2);
	assert(n3->inputs[1]->gate && n4->inputs[1]->gate && n5->outputs[1]->gate);
	assert(n3->inputs[1]->gate == n4->inputs[1]->gate && n3->inputs[1]->gate == n5->outputs[1]->gate);
	assert(n3->inputs[1]->origin == n2->outputs[2]);
	assert(n4->inputs[1]->origin == n2->outputs[2]);
	assert(n6->inputs[1]->origin == n5->outputs[1]);
	
	jive_view(graph, stdout);
	
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}
