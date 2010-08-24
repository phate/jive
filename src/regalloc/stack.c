#include <jive/regalloc/stack.h>
#include <jive/arch/stackframe.h>
#include <jive/arch/registers.h>
#include <jive/context.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/statetype-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/traverser.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>

static void
record_stackslots(jive_graph * graph)
{
	jive_traverser * trav = jive_bottomup_traverser_create(graph);
	
	jive_node * node;
	while( (node = jive_traverser_next(trav)) != 0) {
		size_t n;
		jive_stackframe * stackframe = jive_region_get_stackframe(node->region);
		if (!stackframe) continue;
		for(n=0; n<node->ninputs; n++) {
			jive_stackvar_resource * var = jive_stackvar_resource_cast(node->inputs[n]->resource);
			if (!var) continue;
			if (var->stackframe) continue;
			var->stackframe = stackframe;
			JIVE_LIST_PUSH_BACK(stackframe->vars, var, stackframe_vars_list);
		}
	}
	
	jive_traverser_destroy(trav);
}

static void
layout_regions_recursive(jive_region * region)
{
	if (region->stackframe) region->stackframe->class_->layout(region->stackframe);
	jive_region * subregion;
	JIVE_LIST_ITERATE(region->subregions, subregion, region_subregions_list)
		layout_regions_recursive(subregion);
}

void
jive_regalloc_stack(jive_graph * graph)
{
	record_stackslots(graph);
	layout_regions_recursive(graph->root_region);
}
