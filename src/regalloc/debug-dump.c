/*
 * Copyright 2010 2011 2012 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <stdio.h>

#include <jive/regalloc/shaped-variable.h>
#include <jive/regalloc/xpoint.h>
#include <jive/vsdg.h>

void
jive_shaped_variable_dump(const jive_shaped_variable * self)
{
	fprintf(stderr, "var %p: ", self->variable);
	for (const auto & part : self->interference) {
		jive_shaped_variable * other = part.shaped_variable;
		fprintf(
			stderr,
			"(%p; %p:%zd) ",
			other->variable,
			other->variable->ssavars.first->origin->node,
			other->variable->ssavars.first->origin->index);
	}
	fprintf(stderr, " use_count=%zd\n", self->variable->use_count);
}


void
jive_shaped_node_dump(const jive_shaped_node * shaped_node)
{
	for (const jive_nodevar_xpoint & xpoint : shaped_node->ssavar_xpoints) {
		if (!xpoint.before_count_)
			continue;
		jive_ssavar * ssavar = xpoint.shaped_ssavar->ssavar;
		fprintf(
			stderr,
			"(%p,%p,%p:%zd) ",
			ssavar,
			ssavar->variable,
			ssavar->origin->node,
			ssavar->origin->index);
	}
	fprintf(stderr, "\nNODE %p\n", shaped_node->node);
	for (const jive_nodevar_xpoint & xpoint : shaped_node->ssavar_xpoints) {
		if (!xpoint.after_count_)
			continue;
		jive_ssavar * ssavar = xpoint.shaped_ssavar->ssavar;
		fprintf(
			stderr,
			"(%p,%p,%p:%zd) ",
			ssavar,
			ssavar->variable,
			ssavar->origin->node,
			ssavar->origin->index);
	}
	fprintf(stderr, "\n");
}

void
jive_shaped_region_dump(jive_shaped_graph * shaped_graph, jive_region * region)
{
	jive_region * subregion;
	JIVE_LIST_ITERATE(region->subregions, subregion, region_subregions_list)
		jive_shaped_region_dump(shaped_graph, subregion);
	
	const jive_shaped_region * shaped_region = shaped_graph->map_region(region);
	
	fprintf(stderr, "region %p\n", shaped_region->region);
	for (const jive_cut & cut : shaped_region->cuts()) {
		jive_shaped_node * shaped_node;
		fprintf(stderr, "%p:", &cut);
		for (const jive_shaped_node & shaped_node : cut.nodes()) {
			fprintf(stderr, " %p", shaped_node.node);
		}
		fprintf(stderr, "\n");
	}
}

void
jive_shaped_graph_dump(jive_shaped_graph * shaped_graph)
{
	jive_shaped_region_dump(shaped_graph, shaped_graph->graph->root_region);
}
