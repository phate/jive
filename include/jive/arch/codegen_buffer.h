#ifndef JIVE_ARCH_CODEGEN_BUFFER_H
#define JIVE_ARCH_CODEGEN_BUFFER_H

#include <jive/util/buffer.h>
#include <jive/vsdg/section.h>

typedef struct jive_codegen_buffer jive_codegen_buffer;
typedef struct jive_codegen_buffer_state jive_codegen_buffer_state;

struct jive_codegen_buffer {
	jive_buffer code_buffer;
	jive_buffer data_buffer;
	jive_buffer rodata_buffer;
	jive_buffer bss_buffer;
};

struct jive_codegen_buffer_state {
	size_t code_buffer_size;
	size_t data_buffer_size;
	size_t rodata_buffer_size;
	size_t bss_buffer_size;
};

void
jive_codegen_buffer_init(struct jive_codegen_buffer * self, struct jive_context * context);

void
jive_codegen_buffer_fini(struct jive_codegen_buffer * self);

void
jive_codegen_buffer_save_state(const jive_codegen_buffer * self,
	jive_codegen_buffer_state * state);

void
jive_codegen_buffer_reset(jive_codegen_buffer * self, const jive_codegen_buffer_state * state);

jive_buffer *
jive_codegen_buffer_get_buffer(struct jive_codegen_buffer * self, jive_section section);

void *
jive_codegen_buffer_map_to_memory(const jive_codegen_buffer * self);

#endif
