#include <jive/regalloc/shape.h>
#include <jive/regalloc/shaping-traverser.h>
#include <jive/regalloc/active-place-tracker-private.h>
#include <jive/vsdg/crossings-private.h>

#include <jive/vsdg.h>

static const jive_regcls *
check_crossing_overflow(const jive_node * node,
	jive_resource * add_crossed[const], size_t nadd_crossed,
	jive_resource * remove_crossed[const], size_t nremove_crossed)
{
	jive_regcls_count use_count_before, use_count_after;
	jive_context * context = node->region->graph->context;
	
	const jive_regcls * overflow = 0;
	
	/*
	  Determine if the resource assigned to the first input
	  of this node is passed through this node. For the
	  register allocator this means that it is forbidden
	  to reuse the first input register as output register,
	  while two-operand architectures require exactly that.
	  This is taken care of in later two-op fixup which in
	  turn may need an additional auxiliary register.
	  Detect this case and make sure the register is
	  available.
	*/
	jive_resource * first_input_resource = 0;
	bool first_input_passthrough = false;
	if (node->ninputs) {
		first_input_resource = node->inputs[0]->resource;
		jive_node_resource_interaction * xpoint;
		xpoint = jive_resource_interaction_lookup(&node->resource_interaction, first_input_resource);
		first_input_passthrough = (xpoint->crossed_count != 0);
	}
	
	jive_regcls_count_init(&use_count_before);
	jive_regcls_count_init(&use_count_after);
	
	jive_regcls_count_copy(&use_count_before, context, &node->use_count_before);
	jive_regcls_count_copy(&use_count_after, context, &node->use_count_after);
	
	size_t n;
	for(n=0; n<nremove_crossed; n++) {
		jive_resource * resource = remove_crossed[n];
		jive_node_resource_interaction * xpoint;
		xpoint = jive_node_resource_interaction_lookup(node, resource);
		/* if it was not crossing before, it is not yet accounted for,
		so ignore */
		if (!xpoint->crossed_count) continue;
		
		if (resource == first_input_resource) first_input_passthrough = false;
		
		const jive_regcls * regcls = jive_resource_get_real_regcls(resource);
		
		jive_regcls_count_sub(&use_count_after, context, regcls);
		if (xpoint->before_count == xpoint->crossed_count)
			jive_regcls_count_sub(&use_count_before, context, regcls);
	}
	
	for(n=0; n<nadd_crossed; n++) {
		jive_resource * resource = add_crossed[n];
		jive_node_resource_interaction * xpoint;
		xpoint = jive_node_resource_interaction_lookup(node, resource);
		/* if resource either crossed or defined here, it is
		already accounted for, so ignore */
		if (xpoint && xpoint->after_count) continue;
		
		if (resource == first_input_resource) first_input_passthrough = true;
		
		const jive_regcls * regcls = jive_resource_get_real_regcls(resource);
		
		overflow = jive_regcls_count_add(&use_count_after, context, regcls);
		if (overflow) break;
		
		if (!xpoint || (xpoint->before_count == 0)) {
			overflow = jive_regcls_count_add(&use_count_before, context, regcls);
			if (overflow) break;
		}
	}
	
	/* extra reg for fixup */
	if (!overflow && first_input_passthrough)
		overflow = jive_regcls_count_add(&use_count_before, context, jive_node_get_aux_regcls(node));
	
	jive_regcls_count_fini(&use_count_before, context);
	jive_regcls_count_fini(&use_count_after, context);
	
	return 0;
}

typedef struct jive_region_shaper jive_region_shaper;

struct jive_region_shaper {
	jive_context * context;
	jive_region_shaper * parent;
	jive_region * region;
	jive_shaping_region_traverser * master_traverser;
	jive_traverser * trav;
	jive_active_place_tracker * active;
	
	struct {
		size_t nitems, max;
		jive_active_place ** items;
	} value_priorities;
};

static void
pushdown_node(jive_region_shaper * self, jive_node * new_node);

static void
remove_place_from_prio_list(jive_region_shaper * self, jive_active_place * place)
{
	size_t n = 0;
	while( (n<self->value_priorities.nitems) && (self->value_priorities.items[n] != place) )
		n++;
	n++;
	while( (n<self->value_priorities.nitems) ) {
		self->value_priorities.items[n-1] = self->value_priorities.items[n];
		n++;
	}
}

static void
add_places_to_prio_list(jive_region_shaper * self, size_t index, jive_active_place ** places, size_t nplaces)
{
	if (nplaces + self->value_priorities.nitems > self->value_priorities.max) {
		self->value_priorities.max = (nplaces + self->value_priorities.nitems) * 2;
		self->value_priorities.items = jive_context_realloc(self->context, self->value_priorities.items,
			sizeof(self->value_priorities.items[0]) * self->value_priorities.max);
	}
	
	size_t n = self->value_priorities.nitems;
	while(n>index) {
		n--;
		self->value_priorities.items[n+nplaces] = self->value_priorities.items[n];
	}
	for(n=0; n<nplaces; n++)
		self->value_priorities.items[n+index] = places[n];
	self->value_priorities.nitems += nplaces;
}

static void
jive_region_shaper_fini(jive_region_shaper * self)
{
	jive_active_place_tracker_destroy(self->active);
	jive_context_free(self->context, self->value_priorities.items);
}

static void
jive_region_shaper_init(jive_region_shaper * self, jive_region_shaper * parent, jive_region * region, jive_shaping_region_traverser * master_traverser)
{
	self->context = region->graph->context;
	self->parent = parent;
	self->region = region;
	self->master_traverser = master_traverser;
	self->trav = jive_shaping_region_traverser_enter_region(master_traverser, region);
	self->value_priorities.nitems = self->value_priorities.max = 0;
	self->value_priorities.items = 0;
	if (parent) {
		self->active = jive_active_place_tracker_copy(parent->active);
		self->value_priorities.nitems = self->value_priorities.max = parent->value_priorities.nitems;
		self->value_priorities.items = jive_context_malloc(self->context, sizeof(self->value_priorities.items[0]) * self->value_priorities.nitems);
		size_t n;
		for(n=0; n<self->value_priorities.nitems; n++)
			self->value_priorities.items[n] = parent->value_priorities.items[n];
		/* only take over crossed nodes, discard inputs to the anchor node */
		for(n=0; n<region->anchor_node->ninputs; n++) {
			jive_input * input = region->anchor_node->inputs[n];
			jive_active_place * place = jive_active_place_tracker_get_input_place(self->active, input);
			if (!place) continue;
			jive_active_place_tracker_deactivate_place(self->active, place);
			remove_place_from_prio_list(self, place);
		}
	} else {
		self->active = jive_active_place_tracker_create(self->context);
	}
}

static void
add_inputs_by_priority(jive_region_shaper * self, size_t insertion_index, jive_active_place ** places, size_t nplaces)
{
	size_t n = 0, k = 0;
	
	/* remove non-value places */
	while(n<nplaces) {
		jive_active_place * place = places[n];
		if (jive_output_isinstance(place->origin, &JIVE_VALUE_OUTPUT)) {
			places[k] = place;
			k ++;
		}
		n ++;
	}
	nplaces = k;
	
	/* treat places with lower depth from root with priority */
	/* venerable bubble sort, yes... the number of inputs per node is assumed to be low */
	for(n=1; n<nplaces; n++) {
		for(k=n; k; k--) {
			jive_active_place * first = places[k-1];
			jive_active_place * second = places[k];
			if (first->origin->node->depth_from_root > second->origin->node->depth_from_root) {
				places[k-1] = second;
				places[k] = first;
			}
		}
	}
	
	add_places_to_prio_list(self, insertion_index, places, nplaces);
}

static void
trivial_setup_outputs(jive_region_shaper * self, jive_node * node)
{
	jive_active_place * places[node->noutputs];
	size_t nplaces = 0, n;
	for(n=0; n<node->noutputs; n++) {
		jive_output * output = node->outputs[n];
		jive_active_place * place = jive_active_place_tracker_get_output_place(self->active, output);
		jive_resource * new_constraint = jive_output_get_constraint(output);
		
		place = jive_active_place_tracker_merge_output_constraint(self->active, place, output, new_constraint);
		places[nplaces ++] = place;
	}
	
	for(n=0; n<nplaces; n++) {
		remove_place_from_prio_list(self, places[n]);
		jive_active_place_tracker_deactivate_place(self->active, places[n]);
	}
}

static void
setup_outputs(jive_region_shaper * self, jive_node * node)
{
	/* TODO: implement setup_outputs with splitting support */
	trivial_setup_outputs(self, node);
}

static void
trivial_setup_inputs(jive_region_shaper * self, jive_node * node, size_t insertion_index)
{
	jive_active_place * places[node->ninputs];
	size_t nplaces = 0, n;
	for(n=0; n<node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		jive_active_place * place = jive_active_place_tracker_get_input_place(self->active, input);
		jive_resource * new_constraint = jive_input_get_constraint(input);
		
		place = jive_active_place_tracker_merge_input_constraint(self->active, place, input, new_constraint);
		jive_resource_set_hovering_region(place->resource, self->region);
		places[nplaces ++] = place;
	}
	
	add_inputs_by_priority(self, insertion_index, places, nplaces);
}

static void
setup_inputs(jive_region_shaper * self, jive_node * node, size_t insertion_index)
{
	/* TODO: implement setup_inputs with splitting support */
	trivial_setup_inputs(self, node, insertion_index);
}


static void
process_node(jive_region_shaper * self, jive_node * node)
{
	setup_outputs(self, node);
	setup_inputs(self, node, 0);
	jive_active_place_tracker_unlock(self->active);
	pushdown_node(self, node);
}

static bool
can_move_below_cut(jive_cut * cut, jive_node * new_node,
	jive_resource * inres[const], size_t ninres,
	jive_resource * outres[const], size_t noutres);

static bool
can_move_below_region(jive_region * region, jive_node * new_node,
	jive_resource * inres[const], size_t ninres,
	jive_resource * outres[const], size_t noutres)
{
	jive_cut * cut;
	JIVE_LIST_ITERATE(region->cuts, cut, region_cuts_list) {
		if (!can_move_below_cut(cut, new_node, inres, ninres, outres, noutres)) return false;
	}
	return true;
}

static bool
can_move_below_cut(jive_cut * cut, jive_node * new_node,
	jive_resource * inres[const], size_t ninres,
	jive_resource * outres[const], size_t noutres)
{
	jive_node_location * loc;
	JIVE_LIST_ITERATE(cut->nodes, loc, cut_nodes_list) {
		jive_node * node = loc->node;
		jive_region * region;
		JIVE_LIST_ITERATE(node->anchored_regions, region, node_anchored_regions_list)
			if (!can_move_below_region(region, new_node, inres, ninres, outres, noutres)) return false;
		
		size_t n;
		for(n=0; n<node->ninputs; n++)
			if (node->inputs[n]->origin->node == new_node) return false;
		
		if (check_crossing_overflow(node, inres, ninres, outres, noutres)) return false;
	}
	
	return true;
}

static void
pushdown_node(jive_region_shaper * self, jive_node * new_node)
{
	jive_cut * allowed_cut = 0;
	jive_region * region = self->region;
	
	/* determine set of input/output resources of the new
	node. For unshaped nodes, these sets equal the set
	of resource active before/after the node since there
	cannot yet be any resource crossings */
	size_t ninres = 0, noutres = 0;
	jive_resource * inres[new_node->ninputs];
	jive_resource * outres[new_node->noutputs];
	
	jive_resource_interaction_iterator i;
	JIVE_HASH_ITERATE(jive_resource_interaction, new_node->resource_interaction, i)
		if (i.entry->before_count) inres[ninres++] = i.entry->resource;
	JIVE_HASH_ITERATE(jive_resource_interaction, new_node->resource_interaction, i)
		if (i.entry->after_count) outres[noutres++] = i.entry->resource;
	
	/* push down node to the lowest possible cut */
	jive_cut * cut;
	JIVE_LIST_ITERATE(region->cuts, cut, region_cuts_list) {
		/* test whether required inputs of new node can be passed
		through all nodes of this cut */
		if (!can_move_below_cut(cut, new_node, inres, ninres, outres, noutres)) break;
		
		/* test whether new node can be placed into this cut */
		const jive_regcls * overflow;
		
		/* compute set of resources active after current cut
		(which equals the set of resources active before the first node
		of the next cut) */
		jive_node * next_node = cut->region_cuts_list.next->nodes.first->node;
		jive_resource * active_after_cut[next_node->resource_interaction.nitems];
		size_t nactive_after_cut = 0;
		jive_resource_interaction_iterator i;
		JIVE_HASH_ITERATE(jive_resource_interaction, next_node->resource_interaction, i)
			if (i.entry->before_count) active_after_cut[nactive_after_cut++] = i.entry->resource;
		
		overflow = check_crossing_overflow(new_node,
			active_after_cut, nactive_after_cut,
			NULL, 0);
		if (!overflow) allowed_cut = cut;
	}
	
	/* if moving node to any lower cut fails, create new topmost cut */
	if (!allowed_cut || (new_node->anchored_regions.first))
		allowed_cut = jive_region_create_cut(region);
	
	size_t n;
	for(n=0; n<new_node->noutputs; n++) {
		jive_output * output = new_node->outputs[n];
		jive_resource_set_hovering_region(output->resource, 0);
	}
	
	/* FIXME: make sure nothings gets added to a cut containing just a region anchor */
	jive_cut_append(allowed_cut, new_node);
	
	for(n=0; n<new_node->ninputs; n++) {
		jive_input * input = new_node->inputs[n];
		jive_resource_set_hovering_region(input->resource, region);
	}
}

static bool
trivially_shapable(jive_region_shaper * self, jive_node * new_node)
{
	/* FIXME: implement this function */
	return false;
}

static jive_node *
pick_node(jive_region_shaper * self)
{
	jive_traverser * trav = self->trav;
	if (trav->frontier.first == 0) return 0;
	
	/* single node available, must pick this one */
	if (trav->frontier.first == trav->frontier.last)
		return trav->frontier.first->node;
	
	/* it is always safe to pick nodes that do not increase
	register pressure */
	jive_traversal_nodestate * nodestate;
	JIVE_LIST_ITERATE(trav->frontier, nodestate, traverser_node_list) {
		jive_node * node = nodestate->node;
		if (trivially_shapable(self, node)) return node;
	}
	
	/* if no values active, pick node with highest depth from
	root */
	if (self->value_priorities.nitems == 0) {
		jive_node * node = 0;
		size_t depth = 0;
		JIVE_LIST_ITERATE(trav->frontier, nodestate, traverser_node_list) {
			jive_node * candidate = nodestate->node;
			if (candidate->depth_from_root >= depth) {
				node = candidate;
				depth = candidate->depth_from_root;
			}
		}
		return node;
	}
	
	jive_node * node = 0;
	while(!node) {
		node = self->value_priorities.items[0]->origin->node;
		
		jive_traverser * cone = jive_unshaped_downward_cone_traverser_create(node);
		jive_node * cand;
		while( ( cand = jive_traverser_next(cone)) != 0) {
			bool blocked = false;
			size_t n;
			for(n=0; n<cand->noutputs; n++) {
				jive_output * output = cand->outputs[n];
				jive_active_place * place = jive_active_place_tracker_get_output_place(self->active, output);
				if (place && (jive_output_isinstance(output, &JIVE_VALUE_OUTPUT))) {
					remove_place_from_prio_list(self, place);
					add_places_to_prio_list(self, 0, &place, 1);
					blocked = true;
				}
			}
			if (blocked) {
				node = 0;
				break;
			}
		}
		jive_traverser_destroy(cone);
	}
	
	return node;
}

static void
process_subregions(jive_region_shaper * self, jive_node * new_node);

static void
jive_region_shaper_process(jive_region_shaper * self)
{
	for(;;) {
		jive_node * new_node = pick_node(self);
		if (!new_node) break;
		process_node(self, new_node);
		if (new_node->anchored_regions.first) process_subregions(self, new_node);
	}
}

static void
process_subregions(jive_region_shaper * self, jive_node * new_node)
{
	/* TODO: implement */
	abort();
}

void
jive_regalloc_shape(jive_graph * graph)
{
	jive_shaping_region_traverser * master_traverser = jive_shaping_region_traverser_create(graph);
	
	jive_region_shaper shaper;
	jive_region_shaper_init(&shaper, 0, graph->root_region, master_traverser);
	
	jive_region_shaper_process(&shaper);
	
	jive_region_shaper_fini(&shaper);
	
	jive_shaping_region_traverser_destroy(master_traverser);
}

