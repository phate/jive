/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_MEMLAYOUT_H
#define JIVE_ARCH_MEMLAYOUT_H

#include <jive/common.h>
#include <jive/vsdg/basetype.h>

typedef struct jive_memlayout_mapper jive_memlayout_mapper;
typedef struct jive_memlayout_mapper_class jive_memlayout_mapper_class;

typedef struct jive_dataitem_memlayout jive_dataitem_memlayout;

typedef struct jive_record_memlayout jive_record_memlayout;
typedef struct jive_record_memlayout_element jive_record_memlayout_element;

typedef struct jive_union_memlayout jive_union_memlayout;
typedef struct jive_value_type jive_union_memlayout_element;

namespace jive {
namespace rcd {
	struct declaration;
}}

struct jive_union_declaration;

struct jive_dataitem_memlayout {
	size_t total_size;
	size_t alignment;
	/* FIXME: endianness? */
};

struct jive_union_memlayout {
	jive_dataitem_memlayout base;
	const struct jive_union_declaration * decl;
};

struct jive_record_memlayout_element {
	size_t offset, size;
};

struct jive_record_memlayout {
	jive_dataitem_memlayout base;
	const jive::rcd::declaration * decl;
	jive_record_memlayout_element * element;
};

struct jive_record_memlayout;
struct jive_union_declaration;
struct jive_union_memlayout;
struct jive_bitstring_type;
struct jive_address_type;

struct jive_memlayout_mapper_class {
	const jive_record_memlayout *
	(*map_record)(jive_memlayout_mapper * self, const jive::rcd::declaration * decl);
	
	const jive_union_memlayout *
	(*map_union)(jive_memlayout_mapper * self, const struct jive_union_declaration * decl);
	
	const jive_dataitem_memlayout *
	(*map_bitstring)(jive_memlayout_mapper * self, size_t nbits);
	
	const jive_dataitem_memlayout *
	(*map_address)(jive_memlayout_mapper * self);
};

struct jive_memlayout_mapper {
	const jive_memlayout_mapper_class * class_;
};

JIVE_EXPORTED_INLINE const jive_record_memlayout *
jive_memlayout_mapper_map_record(jive_memlayout_mapper * self, const jive::rcd::declaration * decl)
{
	return self->class_->map_record(self, decl);
}

JIVE_EXPORTED_INLINE const jive_union_memlayout *
jive_memlayout_mapper_map_union(jive_memlayout_mapper * self, const struct jive_union_declaration * decl)
{
	return self->class_->map_union(self, decl);
}

JIVE_EXPORTED_INLINE const jive_dataitem_memlayout *
jive_memlayout_mapper_map_bitstring(jive_memlayout_mapper * self, size_t nbits)
{
	return self->class_->map_bitstring(self, nbits);
}

JIVE_EXPORTED_INLINE const jive_dataitem_memlayout *
jive_memlayout_mapper_map_address(jive_memlayout_mapper * self)
{
	return self->class_->map_address(self);
}

const jive_dataitem_memlayout *
jive_memlayout_mapper_map_value_type(jive_memlayout_mapper * self, const struct jive_value_type * type);

#endif
