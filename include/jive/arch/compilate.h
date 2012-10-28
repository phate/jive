/*
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_CODEGEN_BUFFER_H
#define JIVE_ARCH_CODEGEN_BUFFER_H

#include <jive/util/buffer.h>
#include <jive/vsdg/section.h>

typedef struct jive_compilate jive_compilate;

struct jive_compilate {
	jive_buffer code_buffer;
	jive_buffer data_buffer;
	jive_buffer rodata_buffer;
	jive_buffer bss_buffer;
};

void
jive_compilate_init(struct jive_compilate * self, struct jive_context * context);

void
jive_compilate_fini(struct jive_compilate * self);

/**
	\brief Clear compilation object
	\param self compilation object
	
	Clears the contents of the given compilation object, i.e. subsequently
	it behaves as if it were newly allocated (actual buffers allocated
	might be reused as an optimization, though).
*/
void
jive_compilate_clear(jive_compilate * self);

jive_buffer *
jive_compilate_get_buffer(struct jive_compilate * self, jive_section section);

void *
jive_compilate_map_to_memory(const jive_compilate * self);

#endif
