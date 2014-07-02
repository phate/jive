/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_TAC_THREE_ADDRESS_CODE_H
#define JIVE_FRONTEND_TAC_THREE_ADDRESS_CODE_H

#include <stdbool.h>
#include <stddef.h>

#include <string>
#include <vector>

struct jive_buffer;

/* three address code class */

typedef struct jive_three_address_code_attrs jive_three_address_code_attrs;
typedef struct jive_three_address_code_class jive_three_address_code_class;

struct jive_three_address_code_attrs {
	/* empty, need override */
};

class jive_three_address_code {
public:
	virtual ~jive_three_address_code() noexcept;

	virtual std::string debug_string() const = 0;

	const struct jive_three_address_code_class * class_;

	struct jive_basic_block * basic_block;

	std::vector<jive_three_address_code*> operands;

	struct {
		jive_three_address_code * prev;
		jive_three_address_code * next;
	} basic_block_three_address_codes_list;
};

extern const jive_three_address_code_class JIVE_THREE_ADDRESS_CODE;

struct jive_three_address_code_class {
	const struct jive_three_address_code_class * parent;
	const char * name;

	void (*fini)(jive_three_address_code * self);

	void (*get_label)(const jive_three_address_code * self, struct jive_buffer * buffer);

	const struct jive_three_address_code_attrs *
		(*get_attrs)(const jive_three_address_code * self);
};

JIVE_EXPORTED_INLINE void
jive_three_address_code_get_label(const jive_three_address_code * self,
	struct jive_buffer * buffer)
{
	self->class_->get_label(self, buffer);
}

JIVE_EXPORTED_INLINE const struct jive_three_address_code_attrs *
jive_three_address_code_get_attrs(const jive_three_address_code * self)
{
	return self->class_->get_attrs(self);
}

void
jive_three_address_code_destroy(jive_three_address_code * self);

#endif
