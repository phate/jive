#include <jive/regalloc/xpoint-private.h>

#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-node.h>
#include <jive/regalloc/shaped-variable.h>

jive_nodevar_xpoint *
jive_nodevar_xpoint_create(jive_shaped_node * shaped_node, jive_shaped_ssavar * shaped_ssavar)
{
	jive_context * context = shaped_node->shaped_graph->context;
	
	jive_nodevar_xpoint * xpoint = jive_context_malloc(context, sizeof(*xpoint));
	xpoint->shaped_node = shaped_node;
	xpoint->shaped_ssavar = shaped_ssavar;
	xpoint->before_count = 0;
	xpoint->cross_count = 0;
	xpoint->after_count = 0;
	
	jive_nodevar_xpoint_hash_byssavar_insert(&shaped_node->ssavar_xpoints, xpoint);
	jive_nodevar_xpoint_hash_bynode_insert(&shaped_ssavar->node_xpoints, xpoint);
	
	return xpoint;
}

void
jive_nodevar_xpoint_destroy(jive_nodevar_xpoint * xpoint)
{
	jive_shaped_node * shaped_node = xpoint->shaped_node;
	jive_shaped_ssavar * shaped_ssavar = xpoint->shaped_ssavar;
	jive_context * context = shaped_node->shaped_graph->context;
	
	jive_nodevar_xpoint_hash_byssavar_remove(&shaped_node->ssavar_xpoints, xpoint);
	jive_nodevar_xpoint_hash_bynode_remove(&shaped_ssavar->node_xpoints, xpoint);
	
	jive_context_free(context, xpoint);
}

jive_cutvar_xpoint *
jive_cutvar_xpoint_create(jive_shaped_region * shaped_region, jive_shaped_ssavar * shaped_ssavar)
{
	jive_context * context = shaped_region->shaped_graph->context;
	
	jive_cutvar_xpoint * xpoint = jive_context_malloc(context, sizeof(*xpoint));
	xpoint->shaped_region = shaped_region;
	xpoint->shaped_ssavar = shaped_ssavar;
	xpoint->count = 0;
	
	jive_cutvar_xpoint_hash_byssavar_insert(&shaped_region->ssavar_xpoints, xpoint);
	jive_cutvar_xpoint_hash_byregion_insert(&shaped_ssavar->region_xpoints, xpoint);
	
	return xpoint;
}

void
jive_cutvar_xpoint_destroy(jive_cutvar_xpoint * xpoint)
{
	jive_shaped_region * shaped_region = xpoint->shaped_region;
	jive_shaped_ssavar * shaped_ssavar = xpoint->shaped_ssavar;
	jive_context * context = shaped_region->shaped_graph->context;
	
	jive_cutvar_xpoint_hash_byssavar_remove(&shaped_region->ssavar_xpoints, xpoint);
	jive_cutvar_xpoint_hash_byregion_remove(&shaped_ssavar->region_xpoints, xpoint);
	
	jive_context_free(context, xpoint);
}

