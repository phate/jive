/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/memlayout-simple.h>

#include <jive/types/bitstring/type.h>
#include <jive/types/record/rcdtype.h>
#include <jive/types/union/unntype.h>

static jive_record_memlayout *
jive_memlayout_mapper_simple_add_record_(jive::memlayout_mapper_simple * self,
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

static jive_union_memlayout *
jive_memlayout_mapper_simple_add_union_(jive::memlayout_mapper_simple * self,
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
jive_memlayout_mapper_simple_map_record_(jive::memlayout_mapper * self_,
	const jive::rcd::declaration * decl)
{
	jive::memlayout_mapper_simple * self = dynamic_cast<jive::memlayout_mapper_simple*>(self_);

	auto i = self->record_map.find(decl);
	if (i != self->record_map.end())
		return &i->second;

	jive_record_memlayout * layout = jive_memlayout_mapper_simple_add_record_(self, decl);
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
jive_memlayout_mapper_simple_map_union_(jive::memlayout_mapper * self_,
	const jive::unn::declaration * decl)
{
	jive::memlayout_mapper_simple * self = dynamic_cast<jive::memlayout_mapper_simple*>(self_);

	auto i = self->union_map.find(decl);
	if (i != self->union_map.end())
		return &i->second;
	
	jive_union_memlayout * layout = jive_memlayout_mapper_simple_add_union_(self, decl);
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
jive_memlayout_mapper_simple_map_bitstring_(jive::memlayout_mapper * self_, size_t nbits)
{
	jive::memlayout_mapper_simple * self = dynamic_cast<jive::memlayout_mapper_simple*>(self_);

	auto i = self->bitstring_map.find(nbits);
	if (i != self->bitstring_map.end())
		return &i->second;

	jive_dataitem_memlayout layout;
	if (nbits > self->bits_per_word())
		nbits = (nbits + self->bits_per_word() - 1) & ~ (self->bits_per_word() - 1);
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
	if (layout.alignment > self->bytes_per_word())
		layout.alignment = self->bytes_per_word();

	self->bitstring_map[nbits] = layout;
	
	return &self->bitstring_map[nbits];
}

static const jive_dataitem_memlayout *
jive_memlayout_mapper_simple_map_address_(jive::memlayout_mapper * self_)
{
	jive::memlayout_mapper_simple * self = dynamic_cast<jive::memlayout_mapper_simple*>(self_);
	
	return &self->address_layout;
}

const jive_memlayout_mapper_class JIVE_MEMLAYOUT_MAPPER_SIMPLE = {
	map_record : jive_memlayout_mapper_simple_map_record_,
	map_union : jive_memlayout_mapper_simple_map_union_,
	map_bitstring : jive_memlayout_mapper_simple_map_bitstring_,
	map_address : jive_memlayout_mapper_simple_map_address_
};

namespace jive {

memlayout_mapper_simple::~memlayout_mapper_simple()
{
	for (auto i : record_map)
		delete[] i.second.element;
}

memlayout_mapper_simple::memlayout_mapper_simple(size_t bytes_per_word)
	:	memlayout_mapper()
	, bytes_per_word_(bytes_per_word)
{
	class_ = &JIVE_MEMLAYOUT_MAPPER_SIMPLE;
	address_layout.total_size = bytes_per_word;
	address_layout.alignment = bytes_per_word;
}

}
