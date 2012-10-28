/*
 * Copyright 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_BACKEND_I386_RELOCATION_H
#define JIVE_BACKEND_I386_RELOCATION_H

/*
	Relocation type defines for i386. Note that both naming and numbering
	corresponds to ELF conventions, but the ELF values are not used here
	directly because the compiler should at this stage not have a
	dependence on the ELF format.
*/

#include <jive/arch/compilate.h>

static const jive_relocation_type JIVE_R_386_32 = {1};
static const jive_relocation_type JIVE_R_386_PC32 = {2};
static const jive_relocation_type JIVE_R_386_16 = {20};
static const jive_relocation_type JIVE_R_386_PC16 = {21};
static const jive_relocation_type JIVE_R_386_8 = {22};
static const jive_relocation_type JIVE_R_386_PC8 = {23};

bool
jive_i386_process_relocation(
	void * where, size_t max_size, jive_offset offset,
	jive_relocation_type type, jive_offset target, jive_offset value);

#endif
