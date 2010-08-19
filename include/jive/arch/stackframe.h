#ifndef JIVE_ARCH_STACKFRAME_H
#define JIVE_ARCH_STACKFRAME_H

typedef struct jive_stackframe jive_stackframe;

struct jive_value_output;

struct jive_stackframe {
	jive_value_output * stackptr;
};

#endif
