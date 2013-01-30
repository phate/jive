/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/sequence.h>

#include <jive/vsdg.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/notifiers.h>

JIVE_DEFINE_HASH_TYPE(jive_seq_node_hash, jive_seq_node, struct jive_node *, node, hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_seq_region_hash, jive_seq_region, struct jive_region *, region, hash_chain);

void
jive_seq_point_fini_(jive_seq_point * self)
{
	jive_seq_graph * seq_graph = self->seq_region->seq_graph;
	jive_context * context = seq_graph->context;
	jive_context_free(context, self->attached_labels.items);
	JIVE_LIST_REMOVE(seq_graph->points, self, seqpoint_list);
}

const jive_seq_point_class JIVE_SEQ_POINT = {
	.parent = 0,
	.fini = jive_seq_point_fini_
};

static void
jive_seq_node_fini_(jive_seq_point * self_)
{
	jive_seq_node * self = (jive_seq_node *) self_;
	
	jive_seq_point_fini_(&self->base);
	
	jive_seq_graph * seq_graph = self->base. seq_region->seq_graph;
	jive_seq_node_hash_remove(&seq_graph->node_map, self);
}

const jive_seq_point_class JIVE_SEQ_NODE = {
	.parent = &JIVE_SEQ_POINT,
	.fini = jive_seq_node_fini_
};

static jive_seq_point *
jive_seq_node_create(jive_seq_graph * seq, jive_seq_region * seq_region, jive_node * node)
{
	jive_seq_node * self = jive_context_malloc(seq->context, sizeof(*self));
	self->base.class_ = &JIVE_SEQ_NODE;
	
	jive_seq_point_init(&self->base, seq_region);
	self->node = node;
	self->flags = 0;
	jive_seq_node_hash_insert(&seq->node_map, self);
	
	return &self->base;
}

static void
jive_seq_point_attach_label(jive_seq_point * self, jive_label_internal * label, jive_seq_graph * seq_graph)
{
	if (self->attached_labels.nitems == self->attached_labels.space) {
		self->attached_labels.space = self->attached_labels.space * 2 + 1;
		self->attached_labels.items = jive_context_realloc(seq_graph->context,
			self->attached_labels.items,
			self->attached_labels.space * sizeof(self->attached_labels.items[0]));
	}
	self->attached_labels.items[self->attached_labels.nitems++] = label;
}

static bool
is_active_control(jive_input * input)
{
	if (!jive_input_isinstance(input, &JIVE_CONTROL_INPUT))
		return false;
	return ((jive_control_output *)input->origin)->active;
}

static jive_seq_region *
sequentialize_region(jive_seq_graph * seq, jive_seq_point * before, jive_bottomup_region_traverser * region_trav, jive_region * region)
{
	jive_seq_region * seq_region = jive_context_malloc(seq->context, sizeof(*seq_region));
	seq_region->region = region;
	seq_region->seq_graph = seq;
	seq_region->first_point = 0;
	seq_region->last_point = 0;
	jive_seq_region_hash_insert(&seq->region_map, seq_region);
	JIVE_LIST_PUSH_BACK(seq->regions, seq_region, seqregion_list);
	
	jive_traverser * trav = jive_bottomup_region_traverser_get_node_traverser(region_trav, region);
	
	jive_node * node = jive_traverser_next(trav);
	while(node) {
		jive_seq_point * current = jive_seq_node_create(seq, seq_region, node);
		JIVE_LIST_INSERT(seq->points, before, current, seqpoint_list);
		
		size_t n;
		for(n = 0; n < node->ninputs; n++) {
			jive_input * input = node->inputs[n];
			if (jive_input_isinstance(input, &JIVE_ANCHOR_INPUT)) {
				if (n == 0) {
					jive_seq_region * seq_subregion;
					seq_subregion = sequentialize_region(seq, current, region_trav, input->origin->node->region);
					
					if (!seq_region->last_point)
						seq_region->last_point = seq_subregion->last_point;
					
					current = seq_subregion->first_point;
				} else
					sequentialize_region(seq, 0, region_trav, input->origin->node->region);
			}
		}
		
		seq_region->first_point = current;
		if (!seq_region->last_point)
			seq_region->last_point = current;
		
		before = current;
		
		jive_input * control_input = 0;
		for(n = 0; n < node->ninputs; n++) {
			jive_input * input = node->inputs[n];
			if (is_active_control(input)) {
				control_input = input;
				break;
			}
		}
		
		if (control_input) {
			node = control_input->origin->node;
			jive_bottomup_region_traverser_pass(region_trav, node);
		} else
			node = jive_traverser_next(trav);
	}
	
	return seq_region;
}

static void
jive_seq_graph_label_create(void * closure, jive_label_internal * label)
{
	jive_seq_graph * self = (jive_seq_graph *) closure;
	jive_seq_point * seq_point = jive_seq_graph_map_label_internal(self, label);
	jive_seq_point_attach_label(seq_point, label, self);
}

jive_seq_graph *
jive_graph_sequentialize(jive_graph * graph)
{
	jive_context * context = graph->context;
	jive_seq_graph * seq = jive_context_malloc(context, sizeof(*seq));
	seq->context = context;
	seq->graph = graph;
	seq->points.first = 0;
	seq->points.last = 0;
	seq->regions.first = 0;
	seq->regions.last = 0;
	seq->addrs_changed = true;
	
	jive_seq_node_hash_init(&seq->node_map, context);
	jive_seq_region_hash_init(&seq->region_map, context);
	
	jive_bottomup_region_traverser * region_trav = jive_bottomup_region_traverser_create(graph);
	sequentialize_region(seq, 0, region_trav, graph->root_region);
	jive_bottomup_region_traverser_destroy(region_trav);
	
	jive_label_internal * label;
	JIVE_LIST_ITERATE(graph->labels, label, graph_label_list) {
		jive_seq_point * seq_point = jive_seq_graph_map_label_internal(seq, label);
		jive_seq_point_attach_label(seq_point, label, seq);
	}
	
	seq->label_notifier = jive_label_notifier_slot_connect(&graph->on_label_create, jive_seq_graph_label_create, seq);
	
	return seq;
}

jive_seq_node *
jive_seq_graph_map_node(const jive_seq_graph * seq, struct jive_node * node)
{
	return jive_seq_node_hash_lookup(&seq->node_map, node);
}

jive_seq_region *
jive_seq_graph_map_region(const jive_seq_graph * seq, struct jive_region * region)
{
	return jive_seq_region_hash_lookup(&seq->region_map, region);
}

void
jive_seq_graph_destroy(jive_seq_graph * seq)
{
	jive_seq_point * sp, * next_p;
	JIVE_LIST_ITERATE_SAFE(seq->points, sp, next_p, seqpoint_list) {
		jive_seq_point_destroy(sp);
	}
	jive_seq_region * sr, * next_r;
	JIVE_LIST_ITERATE_SAFE(seq->regions, sr, next_r, seqregion_list) {
		jive_seq_region_hash_remove(&seq->region_map, sr);
		jive_context_free(seq->context, sr);
	}
	jive_seq_node_hash_fini(&seq->node_map);
	jive_seq_region_hash_fini(&seq->region_map);
	
	jive_notifier_disconnect(seq->label_notifier);
	
	jive_context_free(seq->context, seq);
}

jive_seq_point *
jive_seq_graph_map_label_internal(const jive_seq_graph * self, const jive_label_internal * label_)
{
	if (jive_label_isinstance((const jive_label *)label_, &JIVE_LABEL_NODE)) {
		const jive_label_node * label = (const jive_label_node *) label_;
		return &jive_seq_graph_map_node(self, label->node)->base;
	} else if (jive_label_isinstance((const jive_label *)label_, &JIVE_LABEL_REGION_START)) {
		const jive_label_region * label = (const jive_label_region *) label_;
		jive_seq_region * seq_region = jive_seq_graph_map_region(self, label->region);
		if (seq_region) {
			return seq_region->first_point;
		} else {
			return 0;
		}
	} else if (jive_label_isinstance((const jive_label *)label_, &JIVE_LABEL_REGION_END)) {
		const jive_label_region * label = (const jive_label_region *) label_;
		jive_seq_region * seq_region = jive_seq_graph_map_region(self, label->region);
		if (seq_region) {
			return seq_region->last_point;
		} else {
			return 0;
		}
	} else {
		/* there must not be other types of internal labels */
		JIVE_DEBUG_ASSERT(false);
		return 0;
	}
}
