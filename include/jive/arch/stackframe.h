#ifndef JIVE_ARCH_STACKFRAME_H
#define JIVE_ARCH_STACKFRAME_H

typedef struct jive_stackframe jive_stackframe;
typedef struct jive_stackframe_class jive_stackframe_class;

struct jive_output;
struct jive_region;

struct jive_stackframe {
	const jive_stackframe_class * class_;
	struct jive_region * region;
	struct jive_output * stackptr;
};

struct jive_stackframe_class {
	const jive_stackframe_class * parent;
	void (*fini)(jive_stackframe * self);
};

extern const jive_stackframe_class JIVE_STACKFRAME_CLASS;

/* TODO: this is a placeholder function; stackframe factory must
be provided by arch implementation, this generic function
will be removed later  */
jive_stackframe *
jive_stackframe_create(struct jive_region * region, struct jive_output * stackptr);

void
jive_stackframe_destroy(jive_stackframe * self);

#endif
