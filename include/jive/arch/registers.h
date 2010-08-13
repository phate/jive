#ifndef JIVE_ARCH_REGISTERS_H
#define JIVE_ARCH_REGISTERS_H

#include <stddef.h>

struct jive_type;
struct jive_graph;

typedef struct jive_cpureg jive_cpureg;
typedef struct jive_regcls jive_regcls;

struct jive_cpureg {
	const jive_regcls * regcls;
	const char name[32];
	size_t index;
	int code;
};

struct jive_regcls {
	const jive_regcls * parent;
	const char name[32];
	size_t nbits;
	const jive_cpureg * regs;
	size_t nregs;
	size_t index;
	size_t int_arithmetic_width;
	size_t loadstore_width;
	size_t depth;
};

static inline const jive_regcls *
jive_regcls_common_ancestor(const jive_regcls * first, const jive_regcls * second)
{
	while(first != second) {
		if (!first) {second = second->parent; continue;};
		if (!second) {first = first->parent; continue;};
		if (first->depth < second->depth) second = second->parent;
		else first = first->parent;
	}
	return first;
}

static inline const jive_regcls *
jive_regcls_intersection(const jive_regcls * first, const jive_regcls * second)
{
	if (!first) return second;
	if (!second) return first;
	
	const jive_regcls * ancestor = jive_regcls_common_ancestor(first, second);
	if (ancestor == first) return second;
	if (ancestor == second) return first;
	return 0;
}

const struct jive_type *
jive_regcls_get_type(const jive_regcls * self);

struct jive_gate *
jive_regcls_create_gate(const jive_regcls * self, struct jive_graph * graph, const char * name);

#endif
