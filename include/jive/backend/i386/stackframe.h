#ifndef JIVE_BACKEND_I386_STACKFRAME_H
#define JIVE_BACKEND_I386_STACKFRAME_H

#include <jive/arch/stackframe.h>

typedef struct jive_i386_stackframe jive_i386_stackframe;
struct jive_region;
struct jive_output;

struct jive_i386_stackframe {
	jive_stackframe base;
	ssize_t size;
};

extern const jive_stackframe_class JIVE_I386_STACKFRAME_CLASS;

jive_stackframe *
jive_i386_stackframe_create(struct jive_region * region, struct jive_output * stackptr);

#endif
