#ifndef JIVE_REGALLOC_SHAPED_VARIABLE_PRIVATE_H
#define JIVE_REGALLOC_SHAPED_VARIABLE_PRIVATE_H

#include <stdlib.h>

#include <jive/context.h>
#include <jive/regalloc/shaped-variable.h>

struct jive_variable_interference_part {
	jive_shaped_variable * shaped_variable;
	struct {
		jive_variable_interference_part * prev;
		jive_variable_interference_part * next;
	} chain;
	jive_variable_interference * whole;
};

struct jive_variable_interference {
	jive_variable_interference_part first;
	jive_variable_interference_part second;
	size_t count;
};

JIVE_DEFINE_HASH_TYPE(jive_variable_interference_hash, struct jive_variable_interference_part, struct jive_shaped_variable *, shaped_variable, chain);

jive_variable_interference *
jive_variable_interference_create(jive_shaped_variable * first, jive_shaped_variable * second);

void
jive_variable_interference_destroy(jive_variable_interference * self);

static inline size_t
jive_variable_interference_add(jive_shaped_variable * first, jive_shaped_variable * second)
{
	jive_variable_interference * i;
	jive_variable_interference_part * part = jive_variable_interference_hash_lookup(&first->interference, second);
	if (part) i = part->whole;
	else {
		i = jive_variable_interference_create(first, second);
	}
	return i->count ++;
}

static inline size_t
jive_variable_interference_remove(jive_shaped_variable * first, jive_shaped_variable * second)
{
	jive_variable_interference * i;
	jive_variable_interference_part * part = jive_variable_interference_hash_lookup(&first->interference, second);
	i = part->whole;
	size_t count = -- (i->count);
	if (!i->count) {
		jive_variable_interference_destroy(i);
	}
	return count;
}

#endif
