/*
 * Copyright 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/label-mapper.h>

#include <stdio.h>

#include <jive/util/hash.h>

const char *
jive_label_name_mapper_map_label(
	jive_label_name_mapper * self,
	const jive_label * label)
{
	if (label->class_ == &JIVE_LABEL_EXTERNAL)
		return jive_label_name_mapper_map_label_external(self, (const jive_label_external *) label);
	else if (label->class_ == &JIVE_LABEL_CURRENT)
		return ".";
	else if (jive_label_isinstance(label, &JIVE_LABEL_INTERNAL))
		return jive_label_name_mapper_map_label_internal(self, (const jive_label_internal *) label);
	
	JIVE_DEBUG_ASSERT(0);
	return NULL;
}


typedef struct jive_named_int_label jive_named_int_label;
typedef struct jive_label_name_mapper_simple jive_label_name_mapper_simple;
typedef struct jive_named_int_label_hash jive_named_int_label_hash;

struct jive_named_int_label {
	const jive_label_internal * label;
	char * name;
	struct {
		jive_named_int_label * prev;
		jive_named_int_label * next;
	} hash_chain;
};

JIVE_DECLARE_HASH_TYPE(jive_named_int_label_hash, jive_named_int_label, const jive_label_internal *, label, hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_named_int_label_hash, jive_named_int_label, const jive_label_internal *, label, hash_chain);

struct jive_label_name_mapper_simple {
	jive_label_name_mapper base;
	jive_named_int_label_hash named_int_labels;
	jive_context * context;
	size_t int_label_seqno;
	
	const jive_symbol_name_pair * pairs;
	size_t npairs;
};

static void
jive_label_name_mapper_simple_destroy_(jive_label_name_mapper * self_)
{
	jive_label_name_mapper_simple * self = (jive_label_name_mapper_simple *) self_;
	
	struct jive_named_int_label_hash_iterator i;
	i = jive_named_int_label_hash_begin(&self->named_int_labels);
	while (i.entry) {
		jive_named_int_label * entry = i.entry;
		jive_named_int_label_hash_iterator_next(&i);
		
		jive_named_int_label_hash_remove(&self->named_int_labels, entry);
		jive_context_free(self->context, entry->name);
		jive_context_free(self->context, entry);
	}
	
	jive_named_int_label_hash_fini(&self->named_int_labels);
	jive_context_free(self->context, self);
}

static const char *
jive_label_name_mapper_simple_map_label_internal_(jive_label_name_mapper * self_, const jive_label_internal * label)
{
	jive_label_name_mapper_simple * self = (jive_label_name_mapper_simple *) self_;
	
	jive_named_int_label * entry = jive_named_int_label_hash_lookup(&self->named_int_labels, label);
	if (entry)
		return entry->name;
	
	entry = jive_context_malloc(self->context, sizeof(*entry));
	entry->label = label;
	if (label->asmname) {
		entry->name = jive_context_strdup(self->context, label->asmname);
	} else {
		char name[80];
		snprintf(name, sizeof(name), ".L%zd", ++self->int_label_seqno);
		entry->name = jive_context_strdup(self->context, name);
	}
	jive_named_int_label_hash_insert(&self->named_int_labels, entry);
	
	return entry->name;
}

static const char *
jive_label_name_mapper_simple_map_label_external_(jive_label_name_mapper * self_, const jive_label_external * label)
{
	return label->asmname;
}

static const jive_label_name_mapper_class JIVE_LABEL_NAME_MAPPER_SIMPLE = {
	.destroy = jive_label_name_mapper_simple_destroy_,
	.map_label_internal = jive_label_name_mapper_simple_map_label_internal_,
	.map_label_external = jive_label_name_mapper_simple_map_label_external_
};

jive_label_name_mapper *
jive_label_name_mapper_simple_create(
	jive_context * context,
	const jive_symbol_name_pair * pairs,
	size_t npairs)
{
	jive_label_name_mapper_simple * mapper;
	mapper = jive_context_malloc(context, sizeof(*mapper));
	mapper->base.class_ = &JIVE_LABEL_NAME_MAPPER_SIMPLE;
	mapper->context = context;
	mapper->int_label_seqno = 0;
	mapper->pairs = pairs;
	mapper->npairs = npairs;
	jive_named_int_label_hash_init(&mapper->named_int_labels, context);
	
	return &mapper->base;
}

typedef struct jive_label_symbol_mapper_simple jive_label_symbol_mapper_simple;

struct jive_label_symbol_mapper_simple {
	jive_label_symbol_mapper base;
	jive_context * context;
};

static void
jive_label_symbol_mapper_simple_destroy_(jive_label_symbol_mapper * self_)
{
	jive_label_symbol_mapper_simple * self = (jive_label_symbol_mapper_simple *) self_;
	jive_context_free(self->context, self);
}

static const struct jive_linker_symbol *
jive_label_symbol_mapper_simple_map_label_external_(jive_label_symbol_mapper * self_, const jive_label_external * label_)
{
	const jive_label_external_symbol * label = (const jive_label_external_symbol *) label_;
	return &label->symbol;
}

static const jive_label_symbol_mapper_class JIVE_LABEL_SYMBOL_MAPPER_SIMPLE = {
	.destroy = jive_label_symbol_mapper_simple_destroy_,
	.map_label_external = jive_label_symbol_mapper_simple_map_label_external_
};

jive_label_symbol_mapper *
jive_label_symbol_mapper_simple_create(jive_context * context)
{
	jive_label_symbol_mapper_simple * mapper;
	mapper = jive_context_malloc(context, sizeof(*mapper));
	mapper->base.class_ = &JIVE_LABEL_SYMBOL_MAPPER_SIMPLE;
	mapper->context = context;
	
	return &mapper->base;
}
