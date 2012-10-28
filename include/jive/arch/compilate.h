/*
 * Copyright 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_COMPILATE_BUFFER_H
#define JIVE_ARCH_COMPILATE_BUFFER_H

#include <stdint.h>

#include <jive/util/buffer.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/section.h>

typedef struct jive_compilate jive_compilate;
typedef struct jive_section jive_section;


/**
	\brief Section of a compilate
*/
struct jive_section {
	jive_stdsectionid id;
	jive_buffer contents;
	struct {
		jive_section * prev;
		jive_section * next;
	} compilate_section_list;
};

struct jive_compilate {
	jive_context * context;
	struct {
		jive_section * first;
		jive_section * last;
	} sections;
};

void
jive_compilate_init(jive_compilate * self, struct jive_context * context);

void
jive_compilate_fini(jive_compilate * self);

/**
	\brief Clear compilation object
	\param self compilation object
	
	Clears the contents of the given compilation object, i.e. subsequently
	it behaves as if it were newly allocated (actual buffers allocated
	might be reused as an optimization, though).
*/
void
jive_compilate_clear(jive_compilate * self);

jive_section *
jive_compilate_get_standard_section(jive_compilate * self,
	jive_stdsectionid id);

jive_buffer *
jive_compilate_get_buffer(jive_compilate * self, jive_stdsectionid section);

void *
jive_compilate_map_to_memory(const jive_compilate * self);

#endif
