#include <jive/simple-regalloc.h>
#include <jive/internal/bitstringstr.h>
#include <jive/instructionclass.h>
#include <jive/graph.h>
#include <jive/constraints.h>
#include <jive/internal/debug.h>

typedef struct _register_assignment register_assignment;

struct _register_assignment {
	jive_value_bits ***value;
};

static register_assignment *
register_assignment_create(jive_graph * graph, const jive_machine * machine)
{
	size_t n, total_regs = 0;
	for(n=0; n<machine->nregcls; n++)
		total_regs += machine->regcls[n].nregs;
	register_assignment * ra;
	
	jive_value_bits ** tmp = jive_malloc(graph, total_regs*sizeof(jive_value_bits *));
	memset(tmp, 0, total_regs*sizeof(jive_value_bits *));
	
	ra = jive_malloc(graph, sizeof(ra));
	for(n=0; n<machine->nregcls; n++) {
		ra->value[n] = tmp;
		tmp += machine->regcls[n].nregs;
	}
	
	return ra;
}

static void allocate_output_regs(jive_node * node, register_assignment * ra)
{
	jive_value * tmp = jive_node_iterate_output_values(node);
	while(tmp) {
		tmp = tmp->next;
	}
}

static void release_output_regs(jive_node * node, register_assignment * ra)
{
}

static void allocate_input_regs(jive_node * node, register_assignment * ra)
{
}

void
jive_simple_regalloc(jive_graph * graph, const jive_machine * machine)
{
	register_assignment * ra = register_assignment_create(graph, machine);
	
	jive_graph_traverser * trav;
	trav = jive_graph_traverse_bottomup(graph);
	
	while(true) {
		jive_node * node = jive_graph_traverse_next(trav);
		if (!node) break;
		
		allocate_output_regs(node, ra);
		release_output_regs(node, ra);
		allocate_input_regs(node, ra);
		
		(void)ra;
	}
	
	jive_graph_traverse_finish(trav);
}
