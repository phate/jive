#include <jive/regalloc/shape.h>
#include <jive/regalloc/shaping-traverser.h>
#include <jive/regalloc/active-place-tracker-private.h>
#include <jive/regalloc/auxnodes.h>
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
		if (!xpoint || !xpoint->crossed_count) continue;
		
		if (resource == first_input_resource) first_input_passthrough = false;
		
		const jive_regcls * regcls = jive_resource_get_real_regcls(resource);
		if (!regcls) continue;
		
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
		if (!regcls) continue;
		
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
	
	return overflow;
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
remove_place_from_prio_list(jive_region_shaper * self, jive_active_place * place)
{
	if (place->priority < 0) return;
	DEBUG_ASSERT(self->value_priorities.items[place->priority] == place);
	self->value_priorities.items[place->priority] = 0;
	place->priority = -1;
}

static void
add_places_to_prio_list(jive_region_shaper * self, size_t index, jive_active_place ** places, size_t nplaces)
{
	if (nplaces + self->value_priorities.nitems > self->value_priorities.max) {
		self->value_priorities.max = (nplaces + self->value_priorities.nitems) * 2;
		self->value_priorities.items = jive_context_realloc(self->context, self->value_priorities.items,
			sizeof(self->value_priorities.items[0]) * self->value_priorities.max);
	}
	
	/* reserve space and fill with zeroes */
	size_t n = self->value_priorities.nitems;
	while(n>index) {
		n--;
		jive_active_place * place = self->value_priorities.items[n];
		self->value_priorities.items[n+nplaces] = place;
		if (place) place->priority = n + nplaces;
	}
	for(n=0; n<nplaces; n++) self->value_priorities.items[n+index] = 0;
	
	/* move values into created gap, but don't move
	any value further back */
	int insertion_point = index;
	for(n=0; n<nplaces; n++) {
		jive_active_place * place = places[n];
		if ((place->priority >= 0) && (place->priority < insertion_point)) continue;
		
		if (place->priority >= 0)
			self->value_priorities.items[place->priority] = 0;
		self->value_priorities.items[insertion_point] = place;
		place->priority = insertion_point;
		insertion_point ++;
	}
	self->value_priorities.nitems += nplaces;
}

static void
compactify_prio_list(jive_region_shaper * self)
{
	size_t n = 0, k = 0;
	while(k<self->value_priorities.nitems) {
		if (self->value_priorities.items[k]) {
			self->value_priorities.items[n] = self->value_priorities.items[k];
			self->value_priorities.items[n]->priority = n;
			n++;
		}
		k++;
	}
	self->value_priorities.nitems = n;
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
			remove_place_from_prio_list(self, place);
			jive_active_place_tracker_deactivate_place(self->active, place);
		}
		compactify_prio_list(self);
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

static jive_active_place *
select_spill(jive_region_shaper * self, jive_conflict conflict)
{
	if (conflict.type == jive_conflict_register_class) {
		size_t n, lock_level;
		for(lock_level = 0; lock_level<2; lock_level ++) {
			for(n=self->value_priorities.nitems; n; n--) {
				jive_active_place * place = self->value_priorities.items[n-1];
				if (!place) continue;
				if (place->locked > lock_level) continue;
				if (!jive_resource_may_spill(place->resource)) continue;
				
				const jive_regcls * regcls = jive_resource_get_regcls(place->resource);
				if (jive_regcls_intersection(regcls, conflict.regcls) != regcls) continue;
				
				return place;
			}
		}
	} else if (conflict.type == jive_conflict_resource_name) {
		/*return jive_active_resource_map_lookup(self->active->resource_map, conflict.resource);*/
		return conflict.place;
	}
	
	return 0;
}

static jive_node*
get_spilling_node(jive_region_shaper * self, jive_active_place * place)
{
	jive_input * user;
	JIVE_LIST_ITERATE(place->origin->users, user, output_users_list) {
		if (jive_node_isinstance(user->node, &JIVE_AUX_SPILL_NODE))
			return user->node;
	}
	
	return jive_aux_spill_node_create(place->origin->node->region,
		jive_resource_get_regcls(place->resource), place->origin);
}

static void
pushdown_node(jive_region_shaper * self, jive_node * new_node);

static void
restore_recursive(jive_region_shaper * self, jive_active_place * place, jive_output * stackslot_state)
{
	jive_node * restore_node = 0;
	jive_output * restore_value = 0;
	
	jive_input * user, * next_user;
	JIVE_LIST_ITERATE_SAFE(place->origin->users, user, next_user, output_users_list) {
		/* must only restore for nodes within this region and
		completed completed sub-regions; since sub-regions
		will after completion have their places merged
		with their parent, the following check ensures
		exactly that */
		if (user->resource != place->resource) continue;
		
		if (!restore_node) {
			const jive_regcls * regcls = jive_resource_get_regcls(place->resource);
			restore_node = jive_aux_restore_node_create(self->region, regcls, stackslot_state);
			restore_value = restore_node->outputs[0];
		}
		
		jive_input_divert_origin(user, restore_value);
	}
	
	if (self->parent) {
		/* since this sub-region is obviously not yet completed,
		parent region has a different place assigned to the
		same value. Note that the "pushdown" of the restore
		node in the parent region will push it below the
		anchor node of this region, after this completes
		there will therefore not be any copy of this value
		be active anywhere */
		jive_active_place * parent_place = jive_active_place_tracker_get_output_place(self->parent->active, place->origin);
		if (parent_place) restore_recursive(self->parent, parent_place, stackslot_state);
	}
	
	if (restore_node) {
		jive_active_place_tracker_divert_origin(self->active, place->origin, restore_value);
		
		trivial_setup_outputs(self, restore_node);
		trivial_setup_inputs(self, restore_node, 0);
		pushdown_node(self, restore_node);
		
	}
}

static jive_node *
do_spill(jive_region_shaper * self, jive_active_place * place)
{
	jive_node * spill_node = get_spilling_node(self, place);
	jive_output * stackslot_state = spill_node->outputs[0];
	restore_recursive(self, place, stackslot_state);
	
	return spill_node;
}

static jive_node *
do_split(jive_region_shaper * self, jive_active_place * split_place)
{
	jive_node * split_node = jive_aux_valuecopy_node_create(self->region,
		jive_resource_get_regcls(split_place->resource), split_place->origin);
	jive_output * split_output = split_node->outputs[0];
	
	jive_input * user, * next_user;
	JIVE_LIST_ITERATE_SAFE(split_place->origin->users, user, next_user, output_users_list) {
		if (user->resource != split_place->resource) continue;
		jive_input_divert_origin(user, split_output);
	}
	
	jive_active_place_tracker_divert_origin(self->active, split_place->origin, split_output);
	
	return split_node;
}

static void
setup_outputs(jive_region_shaper * self, jive_node * node)
{
	/* Setup outputs of node to be shaped.
	
	Makes sure all outputs of the node have an assigned
	register class (if required) and fit within the total
	budget of allowed register class use counts. Conflicts
	are resolved by splitting and/or spilling.*/
	
	jive_output * remaining[node->noutputs];
	jive_node * spills[node->noutputs];
	size_t nremaining = node->noutputs, n;
	for(n=0; n<node->noutputs; n++) {
		jive_output * output = node->outputs[n];
		remaining[n] = output;
		spills[n] = 0;
		jive_active_place * place = jive_active_place_tracker_get_output_place(self->active, output);
		if (place) place->locked = 1;
	}
	
	while(nremaining) {
		nremaining --;
		jive_output * output = remaining[nremaining];
		jive_node * obstructing_spill_node = spills[nremaining];
		
		DEBUG_ASSERT(!output->resource);
		
		jive_active_place * place = jive_active_place_tracker_get_output_place(self->active, output);
		bool was_active = (place != 0);
		jive_resource * new_constraint = jive_output_get_constraint(output);
		/* resolve all conflicts that would be caused by activating this
		output; note that there might be more than one conflict
		(e.g. both "class" and "name" conflict), so iterate until all
		conflicts are resolved */
		jive_conflict conflict = jive_active_place_tracker_check_replacement_conflict(self->active, place, new_constraint);
		while(conflict.type) {
			jive_active_place * to_spill = select_spill(self, conflict);
			
			bool spilled_output = (to_spill->origin->node == node);
			jive_output * spilled_origin = to_spill->origin;
			
			if (spilled_output && spilled_origin->resource) {
				jive_resource_unassign_output(to_spill->resource, spilled_origin);
				jive_value_resource_recompute_regcls((jive_value_resource *)to_spill->resource);
			}
			
			jive_node * spill_node = do_spill(self, to_spill);
			
			if (spilled_output) {
				/* need to revisit this output */
				size_t index;
				for(index=0; index<nremaining; index++)
					if (spilled_origin == remaining[index]) break;
				for(n=index; n; n--) {
					remaining[n] = remaining[n-1];
					spills[n] = spills[n-1];
				}
				remaining[0] = spilled_origin;
				spills[0] = spill_node;
				if (index == nremaining) nremaining++;
			}
			
			conflict = jive_active_place_tracker_check_replacement_conflict(self->active, place, new_constraint);
		}
		/* we are now certain that this # value may be added to the
		active set; activate it, depending on its state (spilled away,
		conflict with register the value is assigned to, or no conflict
		at all) */
		if (obstructing_spill_node) {
			trivial_setup_outputs(self, obstructing_spill_node);
			trivial_setup_inputs(self, obstructing_spill_node, 0);
			
			/* merging with the new constraint prevents the xfer node
			from being pushed down "too much", since we may
			require a more specialized register class as output
			than the xfer node requires as input */
			place = jive_active_place_tracker_get_output_place(self->active, output);
			jive_active_place_tracker_merge_output_constraint(self->active, place, output, new_constraint);
			pushdown_node(self, obstructing_spill_node);
		} else if (jive_active_place_tracker_check_merge_conflict(self->active, place, new_constraint)) {
			jive_node * xfer_node = do_split(self, place);
			trivial_setup_outputs(self, xfer_node);
			trivial_setup_inputs(self, xfer_node, 0);
			/* merging with the new constraint prevents the xfer node
			from being pushed down "too much", since we may
			require a more specialized register class as output
			than the xfer node requires as input */
			place = jive_active_place_tracker_get_output_place(self->active, output);
			jive_active_place_tracker_merge_output_constraint(self->active, place, output, new_constraint);
			pushdown_node(self, xfer_node);
		} else place = jive_active_place_tracker_merge_output_constraint(self->active, place, output, new_constraint);
		
		/* if the output was not active before, lock it hard to prevent it
		from being spilled again; we can do this because we know that an
		individual nodes register requisition must not exceed the total
		available budget */
		if (was_active) place->locked = 1;
		else place->locked = 2;
	}
	
	for(n=0; n<node->noutputs; n++) {
		jive_output * output = node->outputs[n];
		jive_active_place * place = jive_active_place_tracker_get_output_place(self->active, output);
		remove_place_from_prio_list(self, place);
		jive_active_place_tracker_deactivate_place(self->active, place);
	}
}


static bool
undo_input_setup(jive_output * origin, jive_node * node)
{
	size_t count = 0;
	jive_input * user;
	JIVE_LIST_ITERATE(origin->users, user, output_users_list) {
		if (user->node != node) continue;
		jive_resource * resource = user->resource;
		if (resource) {
			jive_resource_unassign_input(resource, user);
			jive_value_resource_recompute_regcls((jive_value_resource *) resource);
			count ++;
		}
	}
	return (count != 0);
}

static void
setup_inputs(jive_region_shaper * self, jive_node * node, size_t insertion_index)
{
	size_t n;
	for(n=0; n<node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		jive_active_place * place = jive_active_place_tracker_get_input_place(self->active, input);
		if (place) place->locked = 1;
	}
	
	bool done = (node->ninputs == 0);
	
	while(!done) {
		done = true;
		
		for(n=0; n<node->ninputs; n++) {
			jive_input * input = node->inputs[n];
			if (input->resource) continue;
			jive_active_place * place = jive_active_place_tracker_get_input_place(self->active, input);
			jive_resource * new_constraint = jive_input_get_constraint(input);
			
			if (jive_active_place_tracker_check_merge_conflict(self->active, place, new_constraint)) {
				if (undo_input_setup(place->origin, node)) done = false;
				do_split(self, place);
				
				/* value is now inactive */
				place = 0;
			}
			
			jive_conflict conflict = jive_active_place_tracker_check_replacement_conflict(self->active, place, new_constraint);
			if (conflict.type) {
				jive_active_place * to_spill = select_spill(self, conflict);
				if (undo_input_setup(to_spill->origin, node)) done = false;
				do_spill(self, to_spill);
			}
			
			place = jive_active_place_tracker_merge_input_constraint(self->active, place, input, new_constraint);
			place->locked = 1;
			jive_resource_set_hovering_region(place->resource, self->region);
		}
		
		if (done) {
			if (jive_active_place_tracker_get_resource_place(self->active, node->inputs[0]->resource)) {
				jive_conflict conflict = jive_active_place_tracker_check_aux_regcls_conflict(self->active, jive_node_get_aux_regcls(node));
				if (conflict.type) {
					jive_active_place * to_spill = select_spill(self, conflict);
					if (undo_input_setup(to_spill->origin, node)) done = false;
					do_spill(self, to_spill);
				}
			}
		}
	}
	
	jive_active_place * places[node->ninputs];
	size_t nplaces = 0;
	
	for(n=0; n<node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		jive_active_place * place = jive_active_place_tracker_get_input_place(self->active, input);
		if (place->locked) {
			places[nplaces++] = place;
			place->locked = 0;
		}
	}
	
	add_inputs_by_priority(self, insertion_index, places, nplaces);
	compactify_prio_list(self);
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
				
				if (place && (place->priority != -1)) {
					jive_active_place * oplace = self->value_priorities.items[0];
					remove_place_from_prio_list(self, oplace);
					add_places_to_prio_list(self, place->priority + 1, &oplace, 1);
					compactify_prio_list(self);
					blocked = true;
				}
			}
			if (blocked) {
				node = 0;
				break;
			}
			node = cand;
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
	jive_context * context = self->region->graph->context;
	size_t nregions = 0, n;
	
	for(n=0; n<new_node->ninputs; n++) {
		jive_input * input = new_node->inputs[n];
		if (!jive_input_isinstance(input, &JIVE_CONTROL_INPUT)) continue;
		nregions ++;
		jive_active_place * place = jive_active_place_tracker_get_input_place(self->active, input);
		jive_active_place_tracker_deactivate_place(self->active, place);
		jive_resource_set_hovering_region(input->resource, 0);
	}
	
	if (!nregions) return;
	
	jive_region_shaper * subshapers[nregions];
	n = 0;
	jive_region * region;
	JIVE_LIST_ITERATE(new_node->anchored_regions, region, node_anchored_regions_list) {
		jive_region_shaper * subshaper = jive_context_malloc(context, sizeof(*subshaper));
		jive_region_shaper_init(subshaper, self, region, self->master_traverser);
		jive_region_shaper_process(subshaper);
		subshapers[n++] = subshaper;
	}
	
	/* TODO: merge resources back */
	
	for(n=0; n<nregions; n++) {
		jive_region_shaper_fini(subshapers[n]);
		jive_context_free(context, subshapers[n]);
	}
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

