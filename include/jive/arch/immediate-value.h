/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_IMMEDIATE_VALUE_H
#define JIVE_ARCH_IMMEDIATE_VALUE_H

#include <string.h>

#include <jive/arch/linker-symbol.h>
#include <jive/arch/registers.h>
#include <jive/types/bitstring/type.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/valuetype.h>

typedef struct jive_immediate jive_immediate;

typedef uint64_t jive_immediate_int;

/* immediates, as represented in the graph */

struct jive_immediate {
	jive_immediate_int offset;
	const struct jive_label * add_label;
	const struct jive_label * sub_label;
	const void * modifier;
};

JIVE_EXPORTED_INLINE void
jive_immediate_init(jive_immediate * self, jive_immediate_int offset,
	const struct jive_label * add_label, const struct jive_label * sub_label, const void * modifier)
{
	self->offset = offset;
	self->add_label = add_label;
	self->sub_label = sub_label;
	self->modifier = modifier;
}

JIVE_EXPORTED_INLINE void
jive_immediate_assign(const jive_immediate * self, jive_immediate * other)
{
	other->offset = self->offset;
	other->add_label = self->add_label;
	other->sub_label = self->sub_label;
	other->modifier = self->modifier;
}

JIVE_EXPORTED_INLINE jive_immediate
jive_immediate_add(const jive_immediate * a, const jive_immediate * b)
{
	jive_immediate tmp;
	const struct jive_label * add1 = a->add_label;
	const struct jive_label * add2 = b->add_label;
	const struct jive_label * sub1 = a->sub_label;
	const struct jive_label * sub2 = b->sub_label;
	
	if (add1 == sub2) {
		add1 = 0;
		sub2 = 0;
	}
	
	if (add2 && sub1) {
		add2 = 0;
		sub1 = 0;
	}
	
	jive_immediate_int offset = a->offset + b->offset;
	
	if ((add1 && add2) || (sub1 && sub2) || (a->modifier || b->modifier)) {
		jive_immediate_init(&tmp, (jive_immediate_int) -1,
			(const struct jive_label *) -1,
			(const struct jive_label *) -1,
			(const void *) -1);
	} else {
		jive_immediate_init(&tmp, offset, add1 ? add1 : add2, sub1 ? sub1 : sub2, NULL);
	}
	
	return tmp;
}

JIVE_EXPORTED_INLINE jive_immediate
jive_immediate_sub(const jive_immediate * a, const jive_immediate * b)
{
	jive_immediate tmp;
	const struct jive_label * add1 = a->add_label;
	const struct jive_label * add2 = b->sub_label;
	const struct jive_label * sub1 = a->sub_label;
	const struct jive_label * sub2 = b->add_label;
	
	if (add1 == sub2) {
		add1 = 0;
		sub2 = 0;
	}
	
	if (add2 && sub1) {
		add2 = 0;
		sub1 = 0;
	}
	
	jive_immediate_int offset = a->offset - b->offset;
	
	if ((add1 && add2) || (sub1 && sub2) || (a->modifier || b->modifier)) {
		jive_immediate_init(&tmp, (jive_immediate_int) -1,
			(const struct jive_label *) -1,
			(const struct jive_label *) -1,
			(const void *) -1);
	} else {
		jive_immediate_init(&tmp, offset, add1 ? add1 : add2, sub1 ? sub1 : sub2, NULL);
	}
	
	return tmp;
}

JIVE_EXPORTED_INLINE jive_immediate
jive_immediate_add_offset(jive_immediate * self, jive_immediate_int offset)
{
	jive_immediate tmp = *self;
	tmp.offset += offset;
	return tmp;
}

JIVE_EXPORTED_INLINE bool
jive_immediate_equals(const jive_immediate * self, const jive_immediate * other)
{
	return
		(self->offset == other->offset) &&
		(self->add_label == other->add_label) &&
		(self->sub_label == other->sub_label) &&
		(self->modifier == other->modifier);
}

JIVE_EXPORTED_INLINE bool
jive_immediate_has_symbols(const jive_immediate * self)
{
	return self->add_label != 0 || self->sub_label != 0 || self->modifier != 0;
}

#endif
