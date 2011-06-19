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

jive_tpoint *
jive_tpoint_create(jive_shaped_region * shaped_region, jive_shaped_ssavar * shaped_ssavar)
{
	jive_context * context = shaped_region->shaped_graph->context;
	
	jive_tpoint * tpoint = jive_context_malloc(context, sizeof(*tpoint));
	tpoint->shaped_region = shaped_region;
	tpoint->shaped_ssavar = shaped_ssavar;
	tpoint->count = 0;
	
	jive_ssavar_tpoint_hash_insert(&shaped_region->ssavar_tpoints, tpoint);
	jive_region_tpoint_hash_insert(&shaped_ssavar->region_tpoints, tpoint);
	
	return tpoint;
}

void
jive_tpoint_destroy(jive_tpoint * tpoint)
{
	jive_shaped_region * shaped_region = tpoint->shaped_region;
	jive_shaped_ssavar * shaped_ssavar = tpoint->shaped_ssavar;
	jive_context * context = shaped_region->shaped_graph->context;
	
	jive_ssavar_tpoint_hash_remove(&shaped_region->ssavar_tpoints, tpoint);
	jive_region_tpoint_hash_remove(&shaped_ssavar->region_tpoints, tpoint);
	
	jive_context_free(context, tpoint);
}

