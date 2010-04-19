#include <jive/graph.h>
#include <jive/machine.h>
#include <jive/fixed.h>
#include <jive/internal/instruction_str.h>
#include <jive/internal/subroutinestr.h>

#include <jive/i386/machine.h>

struct replacement_map {
	const jive_node_class * node_type;
	jive_i386_instruction_index instruction_type;
};

static const struct replacement_map map_binary[] = {
	{&JIVE_FIXEDADD, jive_i386_int_add},
	{&JIVE_FIXEDAND, jive_i386_int_and},
	{&JIVE_FIXEDOR, jive_i386_int_or},
	{&JIVE_FIXEDXOR, jive_i386_int_xor},
	{&JIVE_FIXEDMUL, jive_i386_int_mul}
};
static const size_t map_binary_size = sizeof(map_binary)/sizeof(map_binary[0]);


static bool
match_replace_binary_op(jive_graph * graph, jive_node * node)
{
	size_t n;
	for(n=0; n<map_binary_size; n++) {
		if (node->type == map_binary[n].node_type) break;
	}
	if (n>=map_binary_size) return false;
		
	jive_value * value = jive_node_iterate_values(node);
	jive_operand * first = jive_node_iterate_operands(node);
	jive_operand * second = first->next;
	
	jive_node * instr = jive_instruction_create(graph,
		&jive_i386_instructions[map_binary[n].instruction_type],
		(jive_value *[]){first->value, second->value}, 0);
	
	jive_value * replacement = jive_instruction_output(instr, 0);
	
	jive_value_replace(value, replacement);
	
	return true;
}

static const struct replacement_map map_unary[] = {
	{&JIVE_FIXEDNEG, jive_i386_int_neg},
};
static const size_t map_unary_size = sizeof(map_unary)/sizeof(map_unary[0]);

static bool
match_replace_unary_op(jive_graph * graph, jive_node * node)
{
	size_t n;
	for(n=0; n<map_unary_size; n++) {
		if (node->type == map_unary[n].node_type) break;
	}
	if (n>=map_unary_size) return false;
		
	jive_value * value = jive_node_iterate_values(node);
	jive_operand * first = jive_node_iterate_operands(node);
	
	jive_node * instr = jive_instruction_create(graph,
		&jive_i386_instructions[map_unary[n].instruction_type],
		(jive_value *[]){first->value}, 0);
	
	jive_value * replacement = jive_instruction_output(instr, 0);
	
	jive_value_replace(value, replacement);
	
	return true;
}


bool
jive_i386_match_instructions(const jive_machine * machine, jive_graph * graph)
{
	jive_graph_traverser * trav;
	
	trav = jive_graph_traverse_bottomup(graph);
	
	jive_node * node;
	
	while( (node = jive_graph_traverse_next(trav)) != 0) {
		if (match_replace_binary_op(graph, node)) continue;
		if (match_replace_unary_op(graph, node)) continue;
	}
	
	jive_graph_traverse_finish(trav);
	
	return true;
}
