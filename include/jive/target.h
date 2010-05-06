#ifndef JIVE_TARGET_H
#define JIVE_TARGET_H

#include <jive/subroutine.h>
#include <jive/machine.h>
#include <jive/graph.h>

typedef struct jive_target jive_target;

struct jive_target {
	const jive_machine * machine;
	
	jive_subroutine * (*create_subroutine)(
		jive_graph * graph,
		const jive_argument_type arguments[],
		size_t narguments,
		jive_argument_type return_value);
};

const jive_target *
jive_get_default_target(void);

/* compiles graph into executable instructions, returns pointer
to first instruction (which will *probably* be the entry into
the first compiled function) */
void *
jive_target_compile_executable(const jive_target * target, jive_graph * graph);

#endif
