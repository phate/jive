/*
 * Copyright 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/label-mapper.h>

#include <stdio.h>

#include <unordered_map>

typedef struct jive_anon_label jive_anon_label;
typedef struct jive_label_name_mapper_simple jive_label_name_mapper_simple;

struct jive_anon_label {
	const void * symbol;
	std::string name;
};

struct jive_label_name_mapper_simple {
	jive_label_name_mapper base;
	std::unordered_map<const void*, jive_anon_label> anon_labels;
	jive_context * context;
	size_t int_label_seqno;
	
	const jive_symbol_name_pair * pairs;
	size_t npairs;
};

static void
jive_label_name_mapper_simple_destroy_(jive_label_name_mapper * self_)
{
	jive_label_name_mapper_simple * self = (jive_label_name_mapper_simple *) self_;
	delete self;
}

static const char *
jive_label_name_mapper_simple_map_anon_symbol_(
	jive_label_name_mapper * self_,
	const void * symbol)
{
	jive_label_name_mapper_simple * self = (jive_label_name_mapper_simple *) self_;
	
	jive_anon_label entry;
	auto i = self->anon_labels.find(symbol);
	if (i != self->anon_labels.end())
		entry = i->second;
	else {
		entry.symbol = symbol;
		
		char name[80];
		snprintf(name, sizeof(name), ".L%zd", ++self->int_label_seqno);
		entry.name = name;

		self->anon_labels[symbol] = entry;
	}
	
	return entry.name.c_str();
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
	destroy : jive_label_name_mapper_simple_destroy_,
	map_named_symbol : jive_label_name_mapper_simple_map_named_symbol_,
	map_anon_symbol : jive_label_name_mapper_simple_map_anon_symbol_
};

jive_label_name_mapper *
jive_label_name_mapper_simple_create(
	jive_context * context,
	const jive_symbol_name_pair * pairs,
	size_t npairs)
{
	jive_label_name_mapper_simple * mapper = new jive_label_name_mapper_simple;
	mapper->base.class_ = &JIVE_LABEL_NAME_MAPPER_SIMPLE;
	mapper->context = context;
	mapper->int_label_seqno = 0;
	mapper->pairs = pairs;
	mapper->npairs = npairs;

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
	delete self_;
}

static const struct jive_linker_symbol *
jive_label_symbol_mapper_simple_map_label_external_(jive_label_symbol_mapper * self_, const jive_label_external * label_)
{
	const jive_label_external_symbol * label = (const jive_label_external_symbol *) label_;
	return &label->symbol;
}

static const jive_label_symbol_mapper_class JIVE_LABEL_SYMBOL_MAPPER_SIMPLE = {
	destroy : jive_label_symbol_mapper_simple_destroy_,
	map_label_external : jive_label_symbol_mapper_simple_map_label_external_
};

jive_label_symbol_mapper *
jive_label_symbol_mapper_simple_create(jive_context * context)
{
	jive_label_symbol_mapper_simple * mapper = new jive_label_symbol_mapper_simple;
	mapper->base.class_ = &JIVE_LABEL_SYMBOL_MAPPER_SIMPLE;
	mapper->context = context;
	
	return &mapper->base;
}
