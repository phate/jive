/*
 * Copyright 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_LINKER_SYMBOL_H
#define JIVE_ARCH_LINKER_SYMBOL_H

#include <stdbool.h>

#include <jive/common.h>
#include <jive/vsdg/section.h>

typedef struct jive_linker_symbol jive_linker_symbol;
typedef struct jive_linker_symbol_resolver jive_linker_symbol_resolver;
typedef struct jive_linker_symbol_resolver_class jive_linker_symbol_resolver_class;
typedef struct jive_symref jive_symref;

struct jive_linker_symbol {
	int tag;
};

struct jive_linker_symbol_resolver_class {
	bool (*resolve)(
		const jive_linker_symbol_resolver * self,
		const jive_linker_symbol * symbol,
		const void ** addr);
};

struct jive_linker_symbol_resolver {
	const jive_linker_symbol_resolver_class * class_;
};

JIVE_EXPORTED_INLINE bool
jive_linker_symbol_resolver_resolve(
	const jive_linker_symbol_resolver * self,
	const jive_linker_symbol * symbol,
	const void ** addr)
{
	return self->class_->resolve(self, symbol, addr);
}

typedef enum jive_symref_type jive_symref_type;
enum jive_symref_type {
	/* no symbol referenced */
	jive_symref_type_none = 0,
	/* reference to standard section type */
	jive_symref_type_section = 1,
	/* reference to linker_symbol */
	jive_symref_type_linker_symbol = 2
};

struct jive_symref {
	jive_symref_type type;
	union {
		jive_stdsectionid section;
		const jive_linker_symbol * linker_symbol;
	} ref;
};

JIVE_EXPORTED_INLINE jive_symref
jive_symref_none(void)
{
	jive_symref symref;
	symref.type = jive_symref_type_none;
	return symref;
}

JIVE_EXPORTED_INLINE jive_symref
jive_symref_section(jive_stdsectionid section)
{
	jive_symref symref;
	symref.type = jive_symref_type_section;
	symref.ref.section = section;
	return symref;
}

JIVE_EXPORTED_INLINE jive_symref
jive_symref_linker_symbol(const jive_linker_symbol * linker_symbol)
{
	jive_symref symref;
	symref.type = jive_symref_type_linker_symbol;
	symref.ref.linker_symbol = linker_symbol;
	return symref;
}

#endif
