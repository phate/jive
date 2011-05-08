#include <jive/vsdg/sequence.h>

#include <jive/vsdg.h>
#include <jive/vsdg/anchortype.h>

static jive_seq_node *
sequentialize_region(jive_seq_graph * seq, jive_seq_node * before, jive_region_traverser * region_trav, jive_region * region)
{
	jive_traverser * trav = jive_region_traverser_get_node_traverser(region_trav, region);
	
	jive_node * node = jive_traverser_next(trav);
	while(node) {
		jive_seq_node * current = jive_context_malloc(seq->context, sizeof(*current));
		current->node = node;
		JIVE_LIST_INSERT(seq->nodes, before, current, seqnode_list);
		
		size_t n;
		for(n = 0; n < node->ninputs; n++) {
			jive_input * input = node->inputs[n];
			if (jive_input_isinstance(input, &JIVE_ANCHOR_INPUT)) {
				if (n == 0)
					current = sequentialize_region(seq, current, region_trav, input->origin->node->region);
				else
					sequentialize_region(seq, 0, region_trav, input->origin->node->region);
			}
		}
		
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
	
	return before;
}

jive_seq_graph *
jive_graph_sequentialize(jive_graph * graph)
{
	jive_seq_graph * seq = jive_context_malloc(graph->context, sizeof(*seq));
	seq->context = graph->context;
	seq->nodes.first = 0;
	seq->nodes.last = 0;
	
	jive_region_traverser * region_trav = jive_bottomup_region_traverser_create(graph);
	sequentialize_region(seq, 0, region_trav, graph->root_region);
	jive_region_traverser_destroy(region_trav);
	
	return seq;
}

void
jive_seq_graph_destroy(jive_seq_graph * seq)
{
	jive_seq_node * sn, * next;
	JIVE_LIST_ITERATE_SAFE(seq->nodes, sn, next, seqnode_list) {
		jive_context_free(seq->context, sn);
	}
	jive_context_free(seq->context, seq);
}

