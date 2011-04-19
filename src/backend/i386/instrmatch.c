#include <jive/backend/i386/instrmatch.h>

#include <jive/arch/instruction.h>
#include <jive/backend/i386/classifier.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/registerset.h>
#include <jive/bitstring/arithmetic.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/traverser.h>

static void
match_bitbinary(jive_node * node, const jive_regselector * regselector)
{
	const jive_register_class * regcls;
	regcls = jive_regselector_map_output(regselector, node->outputs[0]);
	if (regcls == &jive_i386_regcls[jive_i386_gpr]) {
		const jive_instruction_class * icls = 0;
		const jive_bitbinary_operation_class * cls;
		cls = (const jive_bitbinary_operation_class *) node->class_;
		switch(cls->type) {
			case jive_bitop_code_and:
				icls = &jive_i386_instructions[jive_i386_int_and];
				break;
			case jive_bitop_code_or:
				icls = &jive_i386_instructions[jive_i386_int_or];
				break;
			case jive_bitop_code_xor:
				icls = &jive_i386_instructions[jive_i386_int_xor];
				break;
			case jive_bitop_code_sum:
				icls = &jive_i386_instructions[jive_i386_int_add];
				break;
			case jive_bitop_code_product:
				icls = &jive_i386_instructions[jive_i386_int_mul];
				break;
			default:
				return;
		};
		
		jive_node * add;
		add = (jive_node *) jive_instruction_node_create(node->region,
			&jive_i386_instructions[jive_i386_int_add],
			(jive_output *[]){node->inputs[0]->origin, node->inputs[1]->origin}, NULL);
		jive_output_replace(node->outputs[0], add->outputs[0]);
	}
}

static void
match_single(jive_node * node, const jive_regselector * regselector)
{
	if (jive_node_isinstance(node, &JIVE_BITBINARY_NODE)) {
		match_bitbinary(node, regselector);
	}
}

void
jive_i386_match_instructions(jive_graph * graph, const jive_regselector * regselector)
{
	jive_traverser * trav;
	
	trav = jive_bottomup_traverser_create(graph);
	
	for(;;) {
		jive_node * node = jive_traverser_next(trav);
		if (!node) break;
		match_single(node, regselector);
	}
	
	jive_traverser_destroy(trav);
}

