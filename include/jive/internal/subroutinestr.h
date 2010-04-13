#ifndef JIVE_INTERNAL_SUBROUTINESTR_H
#define JIVE_INTERNAL_SUBROUTINESTR_H

#include <jive/subroutine.h>
#include <jive/graph.h>

typedef struct jive_stackframe_vmt jive_stackframe_vmt;
typedef struct jive_subroutine_vmt jive_subroutine_vmt;

#define JIVE_STACKFRAME_COMMON \
	const jive_stackframe_vmt * vmt; \
	jive_stackslot * first, * last; \
	jive_graph * graph; \
	jive_value * stackpointer;

struct jive_stackframe {
	JIVE_STACKFRAME_COMMON
};

struct jive_stackslot {
	jive_stackslot * prev, * next;
	size_t size;
	ssize_t offset;
	
	struct jive_instruction * first_user, * last_user;
};

struct jive_stackframe_vmt {
	jive_stackslot * (*allocate_slot)(jive_stackframe * frame,
		jive_cpureg_class_t regcls);
	void (*finalize)(jive_stackframe * frame);
};

struct jive_subroutine_vmt {
	jive_value * (*get_parameter)(jive_subroutine * sub,
		size_t index);
	void (*return_value)(jive_subroutine * sub, jive_value * value);
};

#define JIVE_SUBROUTINE_COMMON \
	const jive_subroutine_vmt * vmt; \
	\
	jive_graph * graph; \
	jive_stackframe * stackframe; \
	jive_node * enter, * leave; \
	\
	jive_value * stack_pointer, * frame_pointer; \

struct _jive_subroutine {
	JIVE_SUBROUTINE_COMMON
};

#endif
