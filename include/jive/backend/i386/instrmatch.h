#ifndef JIVE_BACKEND_I386_INSTRMATCH_H
#define JIVE_BACKEND_I386_INSTRMATCH_H

#include <jive/arch/regselector.h>

void
jive_i386_match_instructions(struct jive_graph * graph, const jive_regselector * regselector);

#endif
