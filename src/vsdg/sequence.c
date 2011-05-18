#include <jive/vsdg/sequence.h>

#include <jive/vsdg.h>
#include <jive/vsdg/anchortype.h>

JIVE_DEFINE_HASH_TYPE(jive_seq_node_hash, jive_seq_node, struct jive_node *, node, hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_seq_region_hash, jive_seq_region, struct jive_region *, region, hash_chain);

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
		jive_seq_node * current = jive_context_malloc(seq->context, sizeof(*current));
		current->node = node;
		JIVE_LIST_INSERT(seq->nodes, before, current, seqnode_list);
		jive_seq_node_hash_insert(&seq->node_map, current);
		
		size_t n;
		for(n = 0; n < node->ninputs; n++) {
			jive_input * input = node->inputs[n];
			if (jive_input_isinstance(input, &JIVE_ANCHOR_INPUT)) {
				if (n == 0) {
					jive_seq_region * seq_subregion;
					seq_subregion = sequentialize_region(seq, current, region_trav, input->origin->node->region);
					
					if (!seq_region->first_node)
						seq_region->first_node = seq_subregion->first_node;
					
					current = seq_subregion->last_node;
				} else
					sequentialize_region(seq, 0, region_trav, input->origin->node->region);
			}
		}
		
		if (!seq_region->first_node)
			seq_region->first_node = current;
		
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
	
	jive_seq_node_hash_init(&seq->node_map, context);
	jive_seq_region_hash_init(&seq->region_map, context);
	
	jive_region_traverser * region_trav = jive_bottomup_region_traverser_create(graph);
	sequentialize_region(seq, 0, region_trav, graph->root_region);
	jive_region_traverser_destroy(region_trav);
	
	return seq;
}

void
jive_seq_graph_destroy(jive_seq_graph * seq)
{
	jive_seq_node * sn, * next_n;
	JIVE_LIST_ITERATE_SAFE(seq->nodes, sn, next_n, seqnode_list) {
		jive_seq_node_hash_remove(&seq->node_map, sn);
		jive_context_free(seq->context, sn);
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

