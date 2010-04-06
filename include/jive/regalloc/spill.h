#ifndef JIVE_REGALLOC_SPILL_H
#define JIVE_REGALLOC_SPILL_H

#include <jive/machine.h>
#include <jive/instruction.h>

jive_node *
jive_regalloc_spill(jive_value * value, const jive_machine * machine, jive_stackframe * frame);

jive_value *
jive_regalloc_restore(jive_node * spill, const jive_machine * machine, jive_stackframe * frame);

#endif
