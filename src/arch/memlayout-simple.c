/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/memlayout-simple.h>

#include <jive/types/bitstring/type.h>
#include <jive/types/record/rcdtype.h>
#include <jive/types/union/unntype.h>

void
jive_memlayout_mapper_cached_fini_(jive_memlayout_mapper_cached * self)
{
	for (auto i : self->record_map)
		delete[] i.second.element;
}

jive_record_memlayout *
jive_memlayout_mapper_cached_map_record_(jive_memlayout_mapper_cached * self,
	const jive::rcd::declaration * decl)
{
	auto i = self->record_map.find(decl);
	if (i != self->record_map.end())
		return &i->second;
	else
		return NULL;
}

jive_union_memlayout *
jive_memlayout_mapper_cached_map_union_(jive_memlayout_mapper_cached * self,
	const jive::unn::declaration * decl)
{
	auto i = self->union_map.find(decl);
	if (i != self->union_map.end())
		return &i->second;
	else
		return NULL;
}

struct jive_dataitem_memlayout *
jive_memlayout_mapper_cached_map_bitstring_(jive_memlayout_mapper_cached * self, size_t nbits)
{
	auto i = self->bitstring_map.find(nbits);
	if (i != self->bitstring_map.end())
		return &i->second;
	else
		return nullptr;
}

jive_record_memlayout *
jive_memlayout_mapper_cached_add_record_(jive_memlayout_mapper_cached * self,
	const jive::rcd::declaration * decl)
{
	
	jive_record_memlayout layout;
	layout.decl = decl;
	layout.element = new jive_record_memlayout_element[decl->nelements()];
	size_t n = 0;
	for (n = 0; n < decl->nelements(); n++) {
		layout.element[n].offset = 0;
		layout.element[n].size = 0;
	}
	layout.base.alignment = 1;
	layout.base.total_size = 0;

	self->record_map[decl] = layout;
	return &self->record_map[decl];
}

jive_union_memlayout *
jive_memlayout_mapper_cached_add_union_(jive_memlayout_mapper_cached * self,
	const jive::unn::declaration * decl)
{
	jive_union_memlayout layout;
	layout.decl = decl;
	layout.base.alignment = 1;
	layout.base.total_size = 0;
	self->union_map[decl] = layout;
	return &self->union_map[decl];
}

/* simplistic layouter */

static const struct jive_record_memlayout *
jive_memlayout_mapper_simple_map_record_(jive_memlayout_mapper * self_,
	const jive::rcd::declaration * decl)
{
	jive_memlayout_mapper_simple * self = (jive_memlayout_mapper_simple *) self_;
	jive_record_memlayout * layout = jive_memlayout_mapper_cached_map_record_(&self->base, decl);
	if (layout)
		return layout;
	
	layout = jive_memlayout_mapper_cached_add_record_(&self->base, decl);
	size_t pos = 0, alignment = 1, n;
	for (n = 0; n < decl->nelements(); n++) {
		const jive_dataitem_memlayout * ext = jive_memlayout_mapper_map_value_type(self_,
			&decl->element(n));
		
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
jive_memlayout_mapper_simple_map_union_(jive_memlayout_mapper * self_,
	const jive::unn::declaration * decl)
{
	jive_memlayout_mapper_simple * self = (jive_memlayout_mapper_simple *) self_;
	jive_union_memlayout * layout = jive_memlayout_mapper_cached_map_union_(&self->base, decl);
	if (layout)
		return layout;
	
	layout = jive_memlayout_mapper_cached_add_union_(&self->base, decl);
	size_t size = 0, alignment = 1, n;
	for (n = 0; n < decl->nelements; n++) {
		const jive_dataitem_memlayout * ext = jive_memlayout_mapper_map_value_type(self_,
			decl->elements[n]);
		
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
	
	jive_dataitem_memlayout * item = jive_memlayout_mapper_cached_map_bitstring_(&self->base, nbits);
	if (item)
		return item;
	
	jive_dataitem_memlayout layout;
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
	else
		JIVE_DEBUG_ASSERT(0);

	size_t total_size = nbits / 8;
	layout.total_size = total_size;
	layout.alignment = total_size;
	if (layout.alignment > self->bytes_per_word)
		layout.alignment = self->bytes_per_word;

	self->base.bitstring_map[nbits] = layout;
	
	return &self->base.bitstring_map[nbits];
}

static const jive_dataitem_memlayout *
jive_memlayout_mapper_simple_map_address_(jive_memlayout_mapper * self_)
{
	jive_memlayout_mapper_simple * self = (jive_memlayout_mapper_simple *) self_;
	
	return &self->address_layout;
}

const jive_memlayout_mapper_class JIVE_MEMLAYOUT_MAPPER_SIMPLE = {
	map_record : jive_memlayout_mapper_simple_map_record_,
	map_union : jive_memlayout_mapper_simple_map_union_,
	map_bitstring : jive_memlayout_mapper_simple_map_bitstring_,
	map_address : jive_memlayout_mapper_simple_map_address_
};

void
jive_memlayout_mapper_simple_init(jive_memlayout_mapper_simple * self, size_t bits_per_word)
{
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
