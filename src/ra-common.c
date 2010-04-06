#include <jive/machine.h>
#include <jive/internal/debug.h>
#include "ra-common.h"

static inline void
account_for_register(const jive_machine * machine,
	jive_cpureg_t reg,
	short usage_count[])
{
	uint32_t class_mask = machine->regs[reg].class_mask, n = 0;
	while(class_mask) {
		usage_count[n++] += class_mask & 1;
		class_mask >>=1;
	}
}

static inline void
account_for_register_class(const jive_machine * machine,
	jive_cpureg_class_t regcls,
	short usage_count[])
{
	while(regcls != -1) {
		usage_count[regcls]++;
		regcls = machine->regcls[regcls].parent;
	}
}

void
jive_instruction_list_enlarge(jive_graph * graph, jive_instruction_list * list)
{
	size_t new_size = list->space*2 + 1;
	jive_instruction ** tmp = jive_malloc(graph, new_size * sizeof(jive_instruction *));
	memcpy(tmp, list->items, list->nitems * sizeof(jive_instruction *));
	list->items = tmp;
	list->space = new_size;
}

void
jive_register_list_enlarge(jive_graph * graph, jive_register_list * list)
{
	size_t new_size = list->space*2 + 1;
	jive_value_bits ** tmp = jive_malloc(graph, new_size * sizeof(jive_value_bits *));
	memcpy(tmp, list->items, list->nitems * sizeof(jive_value_bits *));
	list->items = tmp;
	list->space = new_size;
}

void
jive_instruction_heap_push(jive_instruction_heap * heap, jive_instruction * instr)
{
	size_t index = heap->nitems;
	jive_instruction_list_append(heap, instr);
	while(index) {
		size_t parent = (index-1) >> 1;
		if (heap->items[parent]->base.depth_from_root > instr->base.depth_from_root) break;
		heap->items[index] = heap->items[parent];
		index = parent;
	}
	heap->items[index] = instr;
}

jive_instruction *
jive_instruction_heap_pop(jive_instruction_heap * heap)
{
	jive_instruction * front = heap->items[0];
	size_t size = heap->nitems--;
	heap->items[0] = heap->items[size-1];
	size_t index = 0;
	while(1) {
		size_t current = index;
		size_t left = (index<<1) + 1;
		size_t right = left + 1;
		
		if (left<size) {
			if (heap->items[left]->base.depth_from_root > heap->items[current]->base.depth_from_root)
				current=left;
			if (right < size) {
				if (heap->items[right]->base.depth_from_root > heap->items[current]->base.depth_from_root)
					current=right;
			}
		}
		
		if (current == index) break;
		jive_instruction * tmp=heap->items[index];
		heap->items[index] = heap->items[current];
		heap->items[current] = tmp;
		index = current;
	}
	return front;
}

jive_instruction *
jive_instruction_heap_peek(const jive_instruction_heap * heap)
{
	if (!heap->nitems) return 0;
	else return heap->items[0];
}

jive_graphcut *
jive_graphcut_create(jive_graph * graph)
{
	jive_graphcut * cut = jive_malloc(graph, sizeof(jive_graphcut));
	
	cut->upper = cut->lower=0;
	jive_instruction_list_init(&cut->available);
	jive_instruction_list_init(&cut->postponed);
	jive_instruction_list_init(&cut->selected);
	jive_register_list_init(&cut->registers);
	
	return cut;
}

/* initialize the bottom cut of the graph that serves
as starting point for the graph shaper */
jive_graphcut *
jive_graphcut_bottom_alloc(
	jive_graph * graph,
	const jive_machine * machine)
{
	jive_graphcut * bottom = jive_graphcut_create(graph);
	bottom->level = 0;
	bottom->subgraph = graph;
	bottom->block_boundary = false;
	
	jive_input_edge_iterator i;
	JIVE_ITERATE_BOTTOM(i, graph) {
		jive_instruction * instr = (jive_instruction *)i->origin.node;
		instr->ra_state = jive_regalloc_inststate_available;
		jive_instruction_list_append(&bottom->available, instr);
	}
		
	/*
		FIXME: add bottom node
	nodelist_add(&bottom->available, g->bottom, g->alloc);
	((instruction *)g->bottom)->ra_state=RA_STATE_AVAILABLE;
	*/
	
	size_t n;
	for(n=0; n<machine->nregcls; n++) {
		bottom->regs_budget[n] = machine->regcls[n].nregs;
		bottom->regs_used[n] = 0;
		bottom->regs_alive[n] = 0;
		bottom->regs_inherited[n] = 0;
		bottom->regs_passthrough[n] = 0;
		bottom->regs_clobbered[n] = 0;
	}
	
	return bottom;
}

void jive_regalloc_compute_regdelta(
	jive_instruction * current_instruction,
	const jive_machine * machine,
	short input_delta[], short output_delta[],
	short used[], short clobber[])
{
	memset(input_delta, 0, machine->nregcls*sizeof(short));
	memset(output_delta, 0, machine->nregcls*sizeof(short));
	memset(used, 0, machine->nregcls*sizeof(short));
	memset(clobber, 0, machine->nregcls*sizeof(short));
	
	/* check the output registers of this instruction; any register value
	computed by this instruction can be removed from the "active" set,
	for any additional value computed, "clobber" registers must be available */
	jive_value * output;
	for(output = jive_node_iterate_values((jive_node *)current_instruction); output; output = output->next) {
		jive_cpureg_class_t regcls = jive_value_get_cpureg_class(output);
		jive_regalloc_regstate ra_state = jive_value_get_regalloc_regstate(output);
		JIVE_ASSERT(regcls);
		
		/* either: all instructions using this value have been scheduled and
		the node is thus in "ready" state, OR there is no instruction using
		this value and the node has therefore never been seen before and
		must be in "none" state */
		JIVE_ASSERT(ra_state == jive_regalloc_regstate_none || ra_state == jive_regalloc_regstate_readers_done);
		
		if (ra_state==jive_regalloc_regstate_readers_done)
		/* this is a value "computed" by the current instruction; if
		it is referenced by subsequent instructions, it must be in
		the regs_active_below set -- since this is the defining
		instruction, it can now be removed */
			account_for_register_class(machine, regcls, output_delta);
		else
		/* if this value is computed, but never needed, we still
		need to reserve a register for the "garbage" value */
			account_for_register_class(machine, regcls, clobber);
	}
	
	/* Now count input registers, if this instruction is selected,
	they will be added to the regs_active_above set */
	jive_operand * input;
	for(input = jive_node_iterate_operands((jive_node *)current_instruction); input; input = input->next) {
		jive_value * input_reg = input->value;
		jive_cpureg_class_t regcls = jive_value_get_cpureg_class(input_reg);
		
		jive_regalloc_regstate ra_state = jive_value_get_regalloc_regstate(input_reg);
		
		/* marker bit, to discount duplicate registers */
		if (ra_state & jive_regalloc_regstate_temp_marker)
			continue;
		
		JIVE_ASSERT(ra_state == jive_regalloc_regstate_none || ra_state == jive_regalloc_regstate_readers_active || ra_state == jive_regalloc_regstate_newly_active);
		if (ra_state == jive_regalloc_regstate_none) {
			/* count this register */
			account_for_register_class(machine, regcls, input_delta);
		} else if (ra_state == jive_regalloc_regstate_newly_active) {
			/* this register is newly introduced by another instruction
			in the same cut; it is therefore not yet accounted for
			as "passthrough" register, but we need to treat it
			as such because we may not overwrite it before the
			other instruction is finished using it */
			account_for_register_class(machine, regcls, used);
		}
		
		jive_value_set_regalloc_regstate(input_reg, ra_state | jive_regalloc_regstate_temp_marker);
	}
	
	/* clear marker bit */
	for(input = jive_node_iterate_operands((jive_node *)current_instruction); input; input = input->next) {
		jive_value * input_reg = input->value;
		jive_regalloc_regstate ra_state = jive_value_get_regalloc_regstate(input_reg);
		
		jive_value_set_regalloc_regstate(input_reg, ra_state & ~jive_regalloc_regstate_temp_marker);
	}
	
	/* if this instruction reuses input machine registers as output registers,
	we must pay special attention if the input values may not yet
	be overwritten -- in this case, the values to be retained must be
	transferred into a temporary register, thus we may need an additional registers */
	size_t nmapped_regs=current_instruction->icls->flags & jive_instruction_mapped_inreg_1;
	size_t n;
	for(n=0; n<nmapped_regs; n++) {
		jive_operand_bits * input = jive_instruction_input((jive_node *)current_instruction, n);
		
		jive_cpureg_class_t regcls = jive_value_get_cpureg_class((jive_value *)input);
		
		jive_regalloc_regstate ra_state = jive_value_get_regalloc_regstate((jive_value *)input->value);
		/* FIXME: for commutative instructions, may swap parameters */
		
		if (ra_state == jive_regalloc_regstate_readers_active)
			account_for_register_class(machine, regcls, used);
	}
	
	/* now compute the number of used registers as the maximum of output
	and input plus any temporary register */
	for(n=0; n<machine->nregcls; n++) {
		int total = used[n]+input_delta[n];
		if (total < output_delta[n]) used[n] = output_delta[n];
		else {
			used[n] = total;
			clobber[n] -= total-output_delta[n];
			if (clobber[n]<0) clobber[n] = 0;
		}
	}
}




#if 0

/* FIXME: does not belong here anyway */

static void close_subgraph(graph *g, graph *subgraph)
{
	node *upper, *interior, *lower;
	
	graph_visit_subgraph(subgraph, &upper, &interior, &lower);
	
	/* the rules are rather simple: 1. make sure that any register used
	within the subgraph is preserved until the end of the subgraph.
	2. make sure that any state dependency of any interior node is shared
	by the boundary nodes. */
	
	node *tmp=upper;
	while(tmp) {
		node *next=tmp->next_visited;
		if (graph_node_is_instance(tmp, &INSTRUCTION_NODE)) {
			/* state dependency: just add in the entry node */
			edge *e=graph_edge_find(g, tmp, subgraph->top);
			if (!e)
				graph_state_edge_create(g, tmp, subgraph->top);
		} else {
			ASSERT_NODE_TYPE(tmp, VIRTUAL_REGISTER_NODE);
			/* value dependency: first check if node is used by anything
			else besides the subgraph entry node */
			int only_entry=1;
			edge_iterator i;
			ITERATE_OUTPUTS(i, tmp) {
				if (i.node==subgraph->top) continue;
				if (i.node->visitor_data==(void *)3) only_entry=0;
			}
			if (!only_entry) {
				/* make sure defining instruction is
				scheduled before entry into subgraph */
				
				node *instr=graph_node_input(tmp, 0);
				
				edge *e=graph_edge_find(g, instr, subgraph->top);
				if (!e)
					graph_state_edge_create(g, instr, subgraph->top);
				
				/* FIXME: the following is only required for loop nodes */
				
				/* make sure the register is also alive at the end of the loop */
				e=graph_edge_find(g, tmp, subgraph->bottom);
				if (!e)
					graph_state_edge_create(g, tmp, subgraph->bottom);
			}
		}
		tmp->next_visited=0;
		tmp->replacement=0;
		tmp->visitor_data=0;
		tmp=next;
	}
	
	tmp=lower;
	while(tmp) {
		node *next=tmp->next_visited;
		if (graph_node_is_instance(tmp, &INSTRUCTION_NODE)) {
			/* state dependency: just add in the exit node */
			edge *e=graph_edge_find(g, subgraph->bottom, tmp);
			if (!e)
				graph_state_edge_create(g, subgraph->bottom, tmp);
		}
		tmp->next_visited=0;
		tmp->replacement=0;
		tmp->visitor_data=0;
		tmp=next;
	}
	
	tmp=interior;
	while(tmp) {
		node *next=tmp->next_visited;
		if (graph_node_is_instance(tmp, &INSTRUCTION_NODE)) {
			((instruction *)tmp)->subgraph=subgraph;
		}
		tmp->next_visited=0;
		tmp->replacement=0;
		tmp->visitor_data=0;
		tmp=next;
	}
}

static void close_subgraph_recursive(graph *g, graph *subgraph)
{
	close_subgraph(g, subgraph);
	
	graph *tmp=subgraph->child;
	while(tmp) {
		close_subgraph_recursive(g, tmp);
		tmp=tmp->sibling;
	}
}

void regalloc_close_subgraphs(graph *g)
{
	close_subgraph_recursive(g, g);
}

#endif
