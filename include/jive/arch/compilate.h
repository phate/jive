/*
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_CODEGEN_BUFFER_H
#define JIVE_ARCH_CODEGEN_BUFFER_H

#include <jive/util/buffer.h>
#include <jive/vsdg/section.h>

typedef struct jive_compilate jive_compilate;
typedef struct jive_compilate_state jive_compilate_state;

struct jive_compilate {
	jive_buffer code_buffer;
	jive_buffer data_buffer;
	jive_buffer rodata_buffer;
	jive_buffer bss_buffer;
};

struct jive_compilate_state {
	size_t code_buffer_size;
	size_t data_buffer_size;
	size_t rodata_buffer_size;
	size_t bss_buffer_size;
};

void
jive_compilate_init(struct jive_compilate * self, struct jive_context * context);

void
jive_compilate_fini(struct jive_compilate * self);

void
jive_compilate_save_state(const jive_compilate * self,
	jive_compilate_state * state);

void
jive_compilate_reset(jive_compilate * self, const jive_compilate_state * state);

jive_buffer *
jive_compilate_get_buffer(struct jive_compilate * self, jive_section section);

void *
jive_compilate_map_to_memory(const jive_compilate * self);

#endif
