/*
 * Copyright 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_LABEL_MAPPER_H
#define JIVE_ARCH_LABEL_MAPPER_H

#include <jive/arch/linker-symbol.h>
#include <jive/vsdg/label.h>

typedef struct jive_label_external_symbol jive_label_external_symbol;
typedef struct jive_label_name_mapper jive_label_name_mapper;
typedef struct jive_label_name_mapper_class jive_label_name_mapper_class;
typedef struct jive_label_symbol_mapper jive_label_symbol_mapper;
typedef struct jive_label_symbol_mapper_class jive_label_symbol_mapper_class;

struct jive_label_name_mapper_class {
	void (*destroy)(jive_label_name_mapper * self);
	const char * (*map_label_internal)(jive_label_name_mapper * self, const jive_label_internal * label);
	const char * (*map_label_external)(jive_label_name_mapper * self, const jive_label_external * label);
	const char * (*map_named_symbol)(jive_label_name_mapper * self, const jive_linker_symbol * symbol);
	const char * (*map_anon_symbol)(jive_label_name_mapper * self, const void * symbol);
};

struct jive_label_name_mapper {
	const jive_label_name_mapper_class * class_;
};

JIVE_EXPORTED_INLINE void
jive_label_name_mapper_destroy(jive_label_name_mapper * self)
{
	self->class_->destroy(self);
}

JIVE_EXPORTED_INLINE const char *
jive_label_name_mapper_map_label_internal(
	jive_label_name_mapper * self,
	const jive_label_internal * label)
{
	return self->class_->map_label_internal(self, label);
}

JIVE_EXPORTED_INLINE const char *
jive_label_name_mapper_map_label_external(
	jive_label_name_mapper * self,
	const jive_label_external * label)
{
	return self->class_->map_label_external(self, label);
}

JIVE_EXPORTED_INLINE const char *
jive_label_name_mapper_map_named_symbol(
	jive_label_name_mapper * self,
	const jive_linker_symbol * symbol)
{
	return self->class_->map_named_symbol(self, symbol);
}

JIVE_EXPORTED_INLINE const char *
jive_label_name_mapper_map_anon_symbol(
	jive_label_name_mapper * self,
	const void * symbol)
{
	return self->class_->map_anon_symbol(self, symbol);
}

const char *
jive_label_name_mapper_map_label(
	jive_label_name_mapper * self,
	const jive_label * label);

typedef struct jive_symbol_name_pair jive_symbol_name_pair;
struct jive_symbol_name_pair {
	const jive_linker_symbol * symbol;
	const char * name;
};

jive_label_name_mapper *
jive_label_name_mapper_simple_create(
	jive_context * context,
	const jive_symbol_name_pair * pairs,
	size_t npairs);

struct jive_label_symbol_mapper_class {
	void (*destroy)(jive_label_symbol_mapper * self);
	const jive_linker_symbol * (*map_label_external)(
		jive_label_symbol_mapper * self,
		const jive_label_external * label);
};

struct jive_label_symbol_mapper {
	const jive_label_symbol_mapper_class * class_;
};

JIVE_EXPORTED_INLINE void
jive_label_symbol_mapper_destroy(jive_label_symbol_mapper * self)
{
	self->class_->destroy(self);
}

JIVE_EXPORTED_INLINE const struct jive_linker_symbol *
jive_label_symbol_mapper_map_label_external(
	jive_label_symbol_mapper * self,
	const jive_label_external * label)
{
	return self->class_->map_label_external(self, label);
}

/* simple label external that includes the symbol; to be used with simple
mapper below */
struct jive_label_external_symbol {
	jive_label_external base;
	jive_linker_symbol symbol;
};

jive_label_symbol_mapper *
jive_label_symbol_mapper_simple_create(jive_context * contex);

#endif
