#include <jive/vsdg/sequence.h>

#include <jive/vsdg.h>
#include <jive/vsdg/anchortype.h>

JIVE_DEFINE_HASH_TYPE(jive_seq_node_hash, jive_seq_node, struct jive_node *, node, hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_seq_region_hash, jive_seq_region, struct jive_region *, region, hash_chain);

static jive_seq_node *
jive_seq_node_create(jive_seq_graph * seq, jive_node * node)
{
	jive_seq_node * self = jive_context_malloc(seq->context, sizeof(*self));
	self->node = node;
	jive_seq_node_hash_insert(&seq->node_map, self);
	self->size = 0;
	self->address = 0;
	self->attached_labels.items = 0;
	self->attached_labels.nitems = self->attached_labels.space = 0;
	return self;
}

static void
jive_seq_node_destroy(jive_seq_node * self, jive_seq_graph * seq_graph)
{
	jive_seq_node_hash_remove(&seq_graph->node_map, self);
	jive_context_free(seq_graph->context, self->attached_labels.items);
	JIVE_LIST_REMOVE(seq_graph->nodes, self, seqnode_list);
	jive_context_free(seq_graph->context, self);
}

static void
jive_seq_node_attach_label(jive_seq_node * self, jive_label_internal * label, jive_seq_graph * seq_graph)
{
	if (self->attached_labels.nitems == self->attached_labels.space) {
		self->attached_labels.space = self->attached_labels.space * 2 + 1;
		self->attached_labels.items = jive_context_realloc(seq_graph->context,
			self->attached_labels.items,
			self->attached_labels.space * sizeof(self->attached_labels.items[0]));
	}
	self->attached_labels.items[self->attached_labels.nitems++] = label;
}

static jive_seq_region *
sequentialize_region(jive_seq_graph * seq, jive_seq_node * before, jive_region_traverser * region_trav, jive_region * region)
{
	jive_seq_region * seq_region = jive_context_malloc(seq->context, sizeof(*seq_region));
	seq_region->region = region;
	seq_region->first_node = 0;
	seq_region->last_node = 0;
	jive_seq_region_hash_insert(&seq->region_map, seq_region);
	JIVE_LIST_PUSH_BACK(seq->regions, seq_region, seqregion_list);
	
	jive_traverser * trav = jive_region_traverser_get_node_traverser(region_trav, region);
	
	jive_node * node = jive_traverser_next(trav);
	while(node) {
		jive_seq_node * current = jive_seq_node_create(seq, node);
		JIVE_LIST_INSERT(seq->nodes, before, current, seqnode_list);
		
		size_t n;
		for(n = 0; n < node->ninputs; n++) {
			jive_input * input = node->inputs[n];
			if (jive_input_isinstance(input, &JIVE_ANCHOR_INPUT)) {
				if (n == 0) {
					jive_seq_region * seq_subregion;
					seq_subregion = sequentialize_region(seq, current, region_trav, input->origin->node->region);
					
					if (!seq_region->last_node)
						seq_region->last_node = seq_subregion->last_node;
					
					current = seq_subregion->last_node;
				} else
					sequentialize_region(seq, 0, region_trav, input->origin->node->region);
			}
		}
		
		seq_region->first_node = current;
		if (!seq_region->last_node)
			seq_region->last_node = current;
		
		before = current;
		
		jive_input * control_input = 0;
		for(n = 0; n < node->ninputs; n++) {
			jive_input * input = node->inputs[n];
			if (jive_input_isinstance(input, &JIVE_CONTROL_INPUT)) {
				control_input = input;
				break;
			}
		}
		
		if (control_input) {
			node = control_input->origin->node;
			jive_traverser_pass(trav, node);
		} else
			node = jive_traverser_next(trav);
	}
	
	return seq_region;
}

jive_seq_graph *
jive_graph_sequentialize(jive_graph * graph)
{
	jive_context * context = graph->context;
	jive_seq_graph * seq = jive_context_malloc(context, sizeof(*seq));
	seq->context = context;
	seq->nodes.first = 0;
	seq->nodes.last = 0;
	seq->regions.first = 0;
	seq->regions.last = 0;
	seq->addrs_changed = true;
	
	jive_seq_node_hash_init(&seq->node_map, context);
	jive_seq_region_hash_init(&seq->region_map, context);
	
	jive_region_traverser * region_trav = jive_bottomup_region_traverser_create(graph);
	sequentialize_region(seq, 0, region_trav, graph->root_region);
	jive_region_traverser_destroy(region_trav);
	
	jive_label_internal * label;
	JIVE_LIST_ITERATE(graph->labels, label, graph_label_list) {
		jive_seq_node * seq_node = jive_label_internal_get_attach_node(label, seq);
		jive_seq_node_attach_label(seq_node, label, seq);
	}
	
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
	jive_seq_node * sn, * next_n;
	JIVE_LIST_ITERATE_SAFE(seq->nodes, sn, next_n, seqnode_list) {
		jive_seq_node_destroy(sn, seq);
	}
	jive_seq_region * sr, * next_r;
	JIVE_LIST_ITERATE_SAFE(seq->regions, sr, next_r, seqregion_list) {
		jive_seq_region_hash_remove(&seq->region_map, sr);
		jive_context_free(seq->context, sr);
	}
	jive_seq_node_hash_fini(&seq->node_map);
	jive_seq_region_hash_fini(&seq->region_map);
	
	jive_context_free(seq->context, seq);
}

