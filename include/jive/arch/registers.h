#ifndef JIVE_ARCH_REGISTERS_H
#define JIVE_ARCH_REGISTERS_H

typedef struct jive_cpureg jive_cpureg;
typedef struct jive_regcls jive_regcls;

struct jive_cpureg {
	const jive_regcls * regcls;
	const char name[32];
};

struct jive_regcls {
	const jive_regcls * parent;
	unsigned short nregs;
	unsigned short depth;
	const char name[32];
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

#endif
