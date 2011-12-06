#include <jive/arch/memlayout-simple.h>

#include <jive/types/bitstring/type.h>
#include <jive/vsdg/recordtype.h>
#include <jive/vsdg/uniontype.h>

JIVE_DEFINE_HASH_TYPE(jive_memlayout_record_hash, jive_memlayout_record_entry, const struct jive_record_declaration *, decl, hash_chain);

JIVE_DEFINE_HASH_TYPE(jive_memlayout_union_hash, jive_memlayout_union_entry, const struct jive_union_declaration *, decl, hash_chain);

JIVE_DEFINE_RANGEMAP_TYPE(jive_memlayout_bitstring_map, jive_dataitem_memlayout *, NULL);

void
jive_memlayout_mapper_cached_init_(jive_memlayout_mapper_cached * self, jive_context * context)
{
	self->context = context;
	jive_memlayout_record_hash_init(&self->record_hash, context);
	jive_memlayout_union_hash_init(&self->union_hash, context);
	jive_memlayout_bitstring_map_init(&self->bitstring_map);
}

void
jive_memlayout_mapper_cached_fini_(jive_memlayout_mapper_cached * self)
{
	struct jive_memlayout_record_hash_iterator i;
	
	i = jive_memlayout_record_hash_begin(&self->record_hash);
	while (i.entry) {
		jive_memlayout_record_entry * entry = i.entry;
		jive_memlayout_record_hash_iterator_next(&i);
		
		jive_memlayout_record_hash_remove(&self->record_hash, entry);
		jive_context_free(self->context, entry->layout.element);
		jive_context_free(self->context, entry);
		
	}
	
	struct jive_memlayout_union_hash_iterator j;
	j = jive_memlayout_union_hash_begin(&self->union_hash);
	while (j.entry) {
		jive_memlayout_union_entry * entry = j.entry;
		jive_memlayout_union_hash_iterator_next(&j);
		
		jive_memlayout_union_hash_remove(&self->union_hash, entry);
		jive_context_free(self->context, entry);
		
	}
	
	ssize_t k;
	for (k = self->bitstring_map.low; k < self->bitstring_map.high; k++) {
		jive_dataitem_memlayout ** playout = jive_memlayout_bitstring_map_lookup(&self->bitstring_map, k);
		if (*playout)
			jive_context_free(self->context, *playout);
	}
	
	jive_memlayout_record_hash_fini(&self->record_hash);
	jive_memlayout_union_hash_fini(&self->union_hash);
	jive_memlayout_bitstring_map_fini(&self->bitstring_map);
}

jive_record_memlayout *
jive_memlayout_mapper_cached_map_record_(jive_memlayout_mapper_cached * self, const struct jive_record_declaration * decl)
{
	jive_memlayout_record_entry * entry = jive_memlayout_record_hash_lookup(&self->record_hash, decl);
	if (entry)
		return &entry->layout;
	else
		return NULL;
}

jive_union_memlayout *
jive_memlayout_mapper_cached_map_union_(jive_memlayout_mapper_cached * self, const struct jive_union_declaration * decl)
{
	jive_memlayout_union_entry * entry = jive_memlayout_union_hash_lookup(&self->union_hash, decl);
	if (entry)
		return &entry->layout;
	else
		return NULL;
}

struct jive_dataitem_memlayout **
jive_memlayout_mapper_cached_map_bitstring_(jive_memlayout_mapper_cached * self, size_t nbits)
{
	return jive_memlayout_bitstring_map_lookup(&self->bitstring_map, nbits);
}

jive_record_memlayout *
jive_memlayout_mapper_cached_add_record_(jive_memlayout_mapper_cached * self, const struct jive_record_declaration * decl)
{
	jive_memlayout_record_entry * entry;
	entry = jive_context_malloc(self->context, sizeof(*entry));
	
	entry->decl = decl;
	
	entry->layout.decl = decl;
	entry->layout.element = jive_context_malloc(self->context, decl->nelements * sizeof(*entry->layout.element));
	size_t n = 0;
	for (n = 0; n < decl->nelements; n++) {
		entry->layout.element[n].offset = 0;
		entry->layout.element[n].size = 0;
	}
	entry->layout.base.alignment = 1;
	entry->layout.base.total_size = 0;
	
	jive_memlayout_record_hash_insert(&self->record_hash, entry);
	
	return &entry->layout;
}

jive_union_memlayout *
jive_memlayout_mapper_cached_add_union_(jive_memlayout_mapper_cached * self, const struct jive_union_declaration * decl)
{
	jive_memlayout_union_entry * entry;
	entry = jive_context_malloc(self->context, sizeof(*entry));
	
	entry->decl = decl;
	
	entry->layout.decl = decl;
	entry->layout.base.alignment = 1;
	entry->layout.base.total_size = 0;
	
	jive_memlayout_union_hash_insert(&self->union_hash, entry);
	
	return &entry->layout;
}

/* simplistic layouter */

static const struct jive_record_memlayout *
jive_memlayout_mapper_simple_map_record_(jive_memlayout_mapper * self_, const struct jive_record_declaration * decl)
{
	jive_memlayout_mapper_simple * self = (jive_memlayout_mapper_simple *) self_;
	jive_record_memlayout * layout = jive_memlayout_mapper_cached_map_record_(&self->base, decl);
	if (layout)
		return layout;
	
	layout = jive_memlayout_mapper_cached_add_record_(&self->base, decl);
	size_t pos = 0, alignment = 1, n;
	for (n = 0; n < decl->nelements; n++) {
		const jive_dataitem_memlayout * ext = jive_memlayout_mapper_map_value_type(self_, decl->elements[n]);
		
		if (alignment < ext->alignment)
			alignment = ext->alignment;
		
		size_t mask = ext->alignment - 1;
		pos = (pos + mask) & ~mask;
		layout->element[n].offset = pos;
		
		pos = pos + ext->total_size;
	}
	
	pos = (pos + alignment - 1) & ~(alignment - 1);
	layout->base.total_size = pos;
	layout->base.alignment = alignment;
	
	return layout;
}

static const struct jive_union_memlayout *
jive_memlayout_mapper_simple_map_union_(jive_memlayout_mapper * self_, const struct jive_union_declaration * decl)
{
	jive_memlayout_mapper_simple * self = (jive_memlayout_mapper_simple *) self_;
	jive_union_memlayout * layout = jive_memlayout_mapper_cached_map_union_(&self->base, decl);
	if (layout)
		return layout;
	
	layout = jive_memlayout_mapper_cached_add_union_(&self->base, decl);
	size_t size = 0, alignment = 1, n;
	for (n = 0; n < decl->nelements; n++) {
		const jive_dataitem_memlayout * ext = jive_memlayout_mapper_map_value_type(self_, decl->elements[n]);
		
		if (alignment < ext->alignment)
			alignment = ext->alignment;
		
		if (size < ext->total_size)
			size = ext->total_size;
	}
	
	size = (size + alignment - 1) & ~(alignment - 1);
	layout->base.total_size = size;
	layout->base.alignment = alignment;
	
	return layout;
}

static const jive_dataitem_memlayout *
jive_memlayout_mapper_simple_map_bitstring_(jive_memlayout_mapper * self_, size_t nbits)
{
	jive_memlayout_mapper_simple * self = (jive_memlayout_mapper_simple *) self_;
	
	jive_dataitem_memlayout ** layout = jive_memlayout_mapper_cached_map_bitstring_(&self->base, nbits);
	
	if (!*layout) {
		*layout = jive_context_malloc(self->base.context, sizeof(**layout));
		
		if (nbits > self->bits_per_word)
			nbits = (nbits + self->bits_per_word - 1) & ~ (self->bits_per_word - 1);
		else if (nbits <= 8)
			nbits = 8;
		else if (nbits <= 16)
			nbits = 16;
		else if (nbits <= 32)
			nbits = 32;
		else if (nbits <= 64)
			nbits = 64;
		else if (nbits <= 128)
			nbits = 128;
		
		size_t total_size = nbits / 8;
		(*layout)->total_size = total_size;
		(*layout)->alignment = total_size;
		if ((*layout)->alignment > self->bytes_per_word)
			(*layout)->alignment = self->bytes_per_word;
	}
	
	return *layout;
}

static const jive_dataitem_memlayout *
jive_memlayout_mapper_simple_map_address_(jive_memlayout_mapper * self_)
{
	jive_memlayout_mapper_simple * self = (jive_memlayout_mapper_simple *) self_;
	
	return &self->address_layout;
}

const jive_memlayout_mapper_class JIVE_MEMLAYOUT_MAPPER_SIMPLE = {
	.map_record = jive_memlayout_mapper_simple_map_record_,
	.map_union = jive_memlayout_mapper_simple_map_union_,
	.map_bitstring = jive_memlayout_mapper_simple_map_bitstring_,
	.map_address = jive_memlayout_mapper_simple_map_address_
};

void
jive_memlayout_mapper_simple_init(jive_memlayout_mapper_simple * self, jive_context * context, size_t bits_per_word)
{
	jive_memlayout_mapper_cached_init_(&self->base, context);
	self->bits_per_word = bits_per_word;
	self->bytes_per_word = bits_per_word / 8;
	self->base.base.class_ = &JIVE_MEMLAYOUT_MAPPER_SIMPLE;
	
	size_t bytes_per_word = self->bits_per_word / 8;
	self->address_layout.total_size = bytes_per_word;
	self->address_layout.alignment = bytes_per_word;
}

void
jive_memlayout_mapper_simple_fini(jive_memlayout_mapper_simple * self)
{
	jive_memlayout_mapper_cached_fini_(&self->base);
}
