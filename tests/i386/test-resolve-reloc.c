/*
 * Copyright 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <jive/arch/compilate.h>
#include <jive/backend/i386/relocation.h>
#include <jive/context.h>

static const char msg[] = {"Hello world!\n"};
static const char * vrfy = 0;

static void
write_msg(const char * s)
{
	write(1, s, strlen(s));
	vrfy = s;
}

const jive_linker_symbol msg_symbol = {};
const jive_linker_symbol write_msg_symbol = {};

static bool sym_resolve(const jive_linker_symbol_resolver * self,
	const jive_linker_symbol * symbol,
	const void ** addr)
{
	if (symbol == &msg_symbol) {
		*addr = msg;
		return true;
	} else if (symbol == &write_msg_symbol) {
		*addr = (const void *) write_msg;
		return true;
	} else {
		return false;
	}
}

static const jive_linker_symbol_resolver_class sym_resolver_cls = {
	resolve : sym_resolve
};

static const jive_linker_symbol_resolver sym_resolver = {
	class_ : &sym_resolver_cls
};

static int test_main(void)
{
	jive_context * ctx = jive_context_create();
	jive_compilate compilate;
	jive_compilate_init(&compilate, ctx);
	
	jive_section * text = jive_compilate_get_standard_section(&compilate, jive_stdsectionid_code);
	
	static const uint8_t prologue[] = {
		0x55, /* push %ebp */
		0x89, 0xe5, /* movl %esp, %ebp */
		0x83, 0xec, 0x08 /* subl $0x08, %esp */
	};
	jive_section_put(text, prologue, sizeof(prologue));
	
	/* movl $msg, %eax */
	jive_section_putbyte(text, 0xb8);
	int32_t displacement = 0;
	jive_section_put_reloc(text, &displacement, sizeof(displacement),
		JIVE_R_386_32, jive_symref_linker_symbol(&msg_symbol), 0);
	
	static const uint8_t xfer[] = {
		0x89, 0x04, 0x24 /* movl %eax,(%esp) */
	};
	jive_section_put(text, xfer, sizeof(xfer));
	
	/* call write_msg */
	(void) write_msg;
	jive_section_putbyte(text, 0xe8);
	int32_t rel = -4;
	jive_section_put_reloc(text, &rel, sizeof(rel),
		JIVE_R_386_PC32, jive_symref_linker_symbol(&write_msg_symbol), 0);
	
	static const uint8_t epilogue[] = {
		0x83, 0xc4, 0x08, /* addl $0x08, %esp */
		0x5d, /* pop %ebp */
		0xc3 /* ret */
	};
	jive_section_put(text, epilogue, sizeof(epilogue));
	
	jive_compilate_map * map = jive_compilate_load(
		&compilate,
		&sym_resolver,
		jive_i386_process_relocation);
	
	jive_compilate_fini(&compilate);
	assert(jive_context_is_empty(ctx));
	
	void (*function)(void) =
		(void(*)(void)) map->sections[0].base;
	
	function();
	assert(vrfy == msg);
	
	jive_compilate_map_unmap(map);
	jive_compilate_map_destroy(map);
	
	jive_context_destroy(ctx);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("i386/test-resolve-reloc", test_main);
