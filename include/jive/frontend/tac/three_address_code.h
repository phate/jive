/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_TAC_THREE_ADDRESS_CODE_H
#define JIVE_FRONTEND_TAC_THREE_ADDRESS_CODE_H

#include <stdbool.h>
#include <stddef.h>

struct jive_buffer;

/* three address code class */

typedef struct jive_three_address_code jive_three_address_code;
typedef struct jive_three_address_code_attrs jive_three_address_code_attrs;
typedef struct jive_three_address_code_class jive_three_address_code_class;

struct jive_three_address_code_attrs {
	/* empty, need override */
};

struct jive_three_address_code {
	const struct jive_three_address_code_class * class_;

	struct jive_basic_block * basic_block;

	size_t noperands;
	struct jive_three_address_code ** operands;

	struct {
		struct jive_three_address_code * prev;
		struct jive_three_address_code * next;
	} basic_block_three_address_codes_list;
};

extern const jive_three_address_code_class JIVE_THREE_ADDRESS_CODE;

struct jive_three_address_code_class {
	const struct jive_three_address_code_class * parent;
	const char * name;

	void (*fini)(struct jive_three_address_code * self);

	void (*get_label)(const struct jive_three_address_code * self, struct jive_buffer * buffer);

	const struct jive_three_address_code_attrs *
		(*get_attrs)(const struct jive_three_address_code * self);

	struct jive_three_address_code * (*create)(struct jive_basic_block * basic_block,
		const struct jive_three_address_code_attrs * attrs,
		size_t noperands, struct jive_three_address_code * const operands[]);
};

JIVE_EXPORTED_INLINE void
jive_three_address_code_get_label(const struct jive_three_address_code * self,
	struct jive_buffer * buffer)
{
	self->class_->get_label(self, buffer);
}

JIVE_EXPORTED_INLINE const struct jive_three_address_code_attrs *
jive_three_address_code_get_attrs(const struct jive_three_address_code * self)
{
	return self->class_->get_attrs(self);
}

JIVE_EXPORTED_INLINE bool
jive_three_address_code_isinstance(const jive_three_address_code * self,
	const struct jive_three_address_code_class * class_)
{
	const struct jive_three_address_code_class * c = self->class_;
	while(c) {
		if (c == class_)
			return true;
		c = c->parent;
	}
	return false;
}

void
jive_three_address_code_destroy(struct jive_three_address_code * self);

#endif