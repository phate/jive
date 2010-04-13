#include <jive/regalloc/cut.h>
#include <jive/machine.h>
#include <jive/internal/instruction_str.h>

#include "debug.h"

jive_graphcut *
jive_graphcut_create(jive_graph * graph, jive_graphcut * lower)
{
	jive_graphcut * cut = jive_malloc(graph, sizeof(*cut));
	cut->graph = graph;
	
	cut->upper = 0;
	cut->lower = lower;
	cut->first = cut->last = 0;
	
	return cut;
}

/* adds an additional constraint to a value to be stored in a 
register */
static inline void
jive_constrain_value(jive_value * value, jive_cpureg_class_t new_constraint)
{
	jive_cpureg_class_t current = jive_value_get_cpureg_class_shared(value);
	
	if (!current) current = new_constraint;
	else current = jive_cpureg_class_intersect(new_constraint, current);
	
	/* FIXME: need to handle the possibility of "inconsistent"
	constraints */
	DEBUG_ASSERT(current);
	jive_value_set_cpureg_class_shared(value, current);	
}

void
jive_regalloc_add_instruction_between(jive_instruction * instr,
	jive_instruction * prev, jive_instruction * next,
	const jive_machine * machine)
{
	jive_value * value;
	jive_operand * operand;
	
	/* FIXME: move constraint propagation somewhere else */
	/* constrain values defined and used by this instruction */
	for(value = jive_node_iterate_values((jive_node *)instr); value; value=value->next) {
		jive_constrain_value(value, jive_value_get_cpureg_class(value));
	}
	for(operand = jive_node_iterate_operands((jive_node *)instr); operand; operand=operand->next) {
		value = operand->value;
		jive_constrain_value(value, jive_operand_get_cpureg_class(operand));
	}
	/* FIXME: handle "inconsistent" constraints */
	
	if (next) {
		jive_value_multiset_copy(&instr->active_before, &next->active_before);
		/* FIXME: this includes "auxiliary" values */
		jive_regcls_count_copy(instr->use_count_after, next->use_count_before);
		jive_regcls_count_copy(instr->use_count_before, next->use_count_before);
	}
	
	/* remove defined values from set, add used values to set */
	for(value = jive_node_iterate_values((jive_node *)instr); value; value=value->next) {
		size_t count = jive_value_multiset_remove_all(&instr->active_before, value);
		jive_cpureg_class_t regcls = jive_value_get_cpureg_class_shared(value);
		if (count) jive_regcls_count_sub(instr->use_count_before, regcls);
		else jive_regcls_count_add(instr->use_count_after, regcls);
	}
	for(operand = jive_node_iterate_operands((jive_node *)instr); operand; operand=operand->next) {
		value = operand->value;
		size_t count = jive_value_multiset_add(&instr->active_before, value);
		jive_cpureg_class_t regcls = jive_value_get_cpureg_class_shared(value);
		if (!count) jive_regcls_count_add(instr->use_count_before, regcls);
	}
	
	DEBUG_ASSERT(! jive_regcls_count_exceeds_class(instr->use_count_before, machine));
	DEBUG_ASSERT(! jive_regcls_count_exceeds_class(instr->use_count_after, machine));

	instr->prev = prev;
	instr->next = next;
	
	if (prev) prev->next = instr;
	if (next) next->prev = instr;
}

/* FIXME: currently does not modify "upper" nodes */
void
jive_graphcut_add_instruction(jive_graphcut * cut, jive_node * _instruction, const jive_machine * machine)
{
	jive_instruction * instruction = (jive_instruction *) _instruction;
	
	jive_instruction * prev = 0, * next = 0;
	if (cut->last) prev = cut->last;
	if (cut->lower) next = cut->lower->first;
	
	cut->last = instruction;
	if (!cut->first) cut->first = instruction;
	
	jive_regalloc_add_instruction_between(instruction, prev, next, machine);
	
	instruction->cut = cut;
}
