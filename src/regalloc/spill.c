#include <jive/regalloc/spill.h>
#include <jive/subroutine.h>

jive_node *
jive_regalloc_spill(jive_value * value, const jive_machine * machine, jive_stackframe * frame)
{
	jive_stackslot * slot = jive_stackframe_allocate_slot(frame, jive_value_get_cpureg_class(value));
	
	jive_node * node = machine->spill(machine, value, slot, frame);
	
	jive_instruction_use_stackslot(node, slot);
	return node;
}

jive_value *
jive_regalloc_restore(jive_node * spill, const jive_machine * machine, jive_stackframe * frame)
{
	jive_stackslot * slot = jive_instruction_get_stackslot(spill);
	
	jive_value * value = machine->restore(machine, spill->graph, 0, frame);
	jive_instruction_use_stackslot(value->node, slot);
	
	return value;
}
