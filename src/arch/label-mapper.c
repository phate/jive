/*
 * Copyright 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/label-mapper.h>

#include <stdio.h>

#include <jive/util/hash.h>

typedef struct jive_anon_label jive_anon_label;
typedef struct jive_anon_label_hash jive_anon_label_hash;
typedef struct jive_label_name_mapper_simple jive_label_name_mapper_simple;

struct jive_anon_label {
	const void * symbol;
	char * name;
	struct {
		jive_anon_label * prev;
		jive_anon_label * next;
	} hash_chain;
};

JIVE_DECLARE_HASH_TYPE(jive_anon_label_hash, jive_anon_label, const void *, symbol, hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_anon_label_hash, jive_anon_label, const void *, symbol, hash_chain);

struct jive_label_name_mapper_simple {
	jive_label_name_mapper base;
	jive_anon_label_hash anon_labels;
	jive_context * context;
	size_t int_label_seqno;
	
	const jive_symbol_name_pair * pairs;
	size_t npairs;
};

static void
jive_label_name_mapper_simple_destroy_(jive_label_name_mapper * self_)
{
	jive_label_name_mapper_simple * self = (jive_label_name_mapper_simple *) self_;
	
	struct jive_anon_label_hash_iterator j;
	j = jive_anon_label_hash_begin(&self->anon_labels);
	while (j.entry) {
		jive_anon_label * entry = j.entry;
		jive_anon_label_hash_iterator_next(&j);
		
		jive_anon_label_hash_remove(&self->anon_labels, entry);
		jive_context_free(self->context, entry->name);
		jive_context_free(self->context, entry);
	}
	jive_anon_label_hash_fini(&self->anon_labels);
	
	jive_context_free(self->context, self);
}

static const char *
jive_label_name_mapper_simple_map_anon_symbol_(
	jive_label_name_mapper * self_,
	const void * symbol)
{
	jive_label_name_mapper_simple * self = (jive_label_name_mapper_simple *) self_;
	
	jive_anon_label * entry = jive_anon_label_hash_lookup(&self->anon_labels, symbol);
	if (!entry) {
		entry = jive_context_malloc(self->context, sizeof(*entry));
		entry->symbol = symbol;
		
		char name[80];
		snprintf(name, sizeof(name), ".L%zd", ++self->int_label_seqno);
		entry->name = jive_context_strdup(self->context, name);
		
		jive_anon_label_hash_insert(&self->anon_labels, entry);
	}
	
	return entry->name;
}

static const char *
jive_label_name_mapper_simple_map_named_symbol_(
	jive_label_name_mapper * self_,
	const jive_linker_symbol * symbol)
{
	jive_label_name_mapper_simple * self = (jive_label_name_mapper_simple *) self_;
	
	size_t n;
	for (n = 0; n < self->npairs; ++n) {
		if (self->pairs[n].symbol == symbol)
			return self->pairs[n].name;
	}
	
	return 0;
}

static const jive_label_name_mapper_class JIVE_LABEL_NAME_MAPPER_SIMPLE = {
	.destroy = jive_label_name_mapper_simple_destroy_,
	.map_named_symbol = jive_label_name_mapper_simple_map_named_symbol_,
	.map_anon_symbol = jive_label_name_mapper_simple_map_anon_symbol_
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
	jive_anon_label_hash_init(&mapper->anon_labels, context);
	
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
