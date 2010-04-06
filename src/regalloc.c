#include <jive/regalloc.h>
#include <jive/regalloc/shape.h>
#include <jive/regalloc/assign.h>

void
jive_regalloc(jive_graph * graph, const jive_machine * machine, jive_stackframe * frame)
{
	jive_instruction_sequence seq;
	
	jive_regalloc_shape(graph, machine, frame, &seq);
	jive_regalloc_assign(&seq, machine, frame);
}
