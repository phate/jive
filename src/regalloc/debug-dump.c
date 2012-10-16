/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <stdio.h>

#include <jive/regalloc/shaped-variable-private.h>
#include <jive/regalloc/xpoint-private.h>
#include <jive/vsdg.h>

void
jive_shaped_variable_dump(const jive_shaped_variable * self)
{
	fprintf(stderr, "var %p: ", self->variable);
	struct jive_variable_interference_hash_iterator i;
	JIVE_HASH_ITERATE(jive_variable_interference_hash, self->interference, i) {
		jive_shaped_variable * other = i.entry->shaped_variable;
		fprintf(stderr, "(%p; %p:%zd) ", other->variable, other->variable->ssavars.first->origin->node, other->variable->ssavars.first->origin->index);
	}
	fprintf(stderr, " use_count=%zd\n", self->variable->use_count);
}


void
jive_shaped_node_dump(const jive_shaped_node * shaped_node)
{
	struct jive_nodevar_xpoint_hash_byssavar_iterator i;
	
	JIVE_HASH_ITERATE(jive_nodevar_xpoint_hash_byssavar, shaped_node->ssavar_xpoints, i) {
		jive_nodevar_xpoint * xpoint = i.entry;
		if (!xpoint->before_count)
			continue;
		jive_ssavar * ssavar = xpoint->shaped_ssavar->ssavar;
		fprintf(stderr, "(%p,%p,%p:%zd) ", ssavar, ssavar->variable, ssavar->origin->node, ssavar->origin->index);
	}
	fprintf(stderr, "\nNODE %p\n", shaped_node->node);
	JIVE_HASH_ITERATE(jive_nodevar_xpoint_hash_byssavar, shaped_node->ssavar_xpoints, i) {
		jive_nodevar_xpoint * xpoint = i.entry;
		if (!xpoint->after_count)
			continue;
		jive_ssavar * ssavar = xpoint->shaped_ssavar->ssavar;
		fprintf(stderr, "(%p,%p,%p:%zd) ", ssavar, ssavar->variable, ssavar->origin->node, ssavar->origin->index);
	}
	fprintf(stderr, "\n");
}

void
jive_shaped_region_dump(jive_shaped_graph * shaped_graph, jive_region * region)
{
	jive_region * subregion;
	JIVE_LIST_ITERATE(region->subregions, subregion, region_subregions_list)
		jive_shaped_region_dump(shaped_graph, subregion);
	
	const jive_shaped_region * shaped_region = jive_shaped_graph_map_region(shaped_graph, region);
	
	fprintf(stderr, "region %p\n", shaped_region->region);
	jive_cut * cut;
	JIVE_LIST_ITERATE(shaped_region->cuts, cut, region_cut_list) {
		jive_shaped_node * shaped_node;
		fprintf(stderr, "%p:", cut);
		JIVE_LIST_ITERATE(cut->locations, shaped_node, cut_location_list)
			fprintf(stderr, " %p", shaped_node->node);
		fprintf(stderr, "\n");
	}
}

void
jive_shaped_graph_dump(jive_shaped_graph * shaped_graph)
{
	jive_shaped_region_dump(shaped_graph, shaped_graph->graph->root_region);
}
