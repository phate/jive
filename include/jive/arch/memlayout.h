/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_MEMLAYOUT_H
#define JIVE_ARCH_MEMLAYOUT_H

#include <jive/common.h>
#include <jive/vsdg/basetype.h>

namespace jive {

class memlayout_mapper;

namespace rcd {
	struct declaration;
}
namespace unn {
	struct declaration;
}
namespace value {
	class type;
}
}

typedef struct jive_memlayout_mapper_class jive_memlayout_mapper_class;

typedef struct jive_dataitem_memlayout jive_dataitem_memlayout;

typedef struct jive_record_memlayout jive_record_memlayout;
typedef struct jive_record_memlayout_element jive_record_memlayout_element;

typedef struct jive_union_memlayout jive_union_memlayout;
typedef jive::value::type jive_union_memlayout_element;

struct jive_dataitem_memlayout {
	size_t total_size;
	size_t alignment;
	/* FIXME: endianness? */
};

struct jive_union_memlayout {
	jive_dataitem_memlayout base;
	const struct jive::unn::declaration * decl;
};

struct jive_record_memlayout_element {
	size_t offset, size;
};

struct jive_record_memlayout {
	jive_dataitem_memlayout base;
	const jive::rcd::declaration * decl;
	jive_record_memlayout_element * element;
};

struct jive_address_type;
struct jive_bitstring_type;
struct jive_union_declaration;

namespace jive {

class memlayout_mapper {
public:
	virtual
	~memlayout_mapper();

	inline constexpr
	memlayout_mapper(size_t bytes_per_word)
		: bytes_per_word_(bytes_per_word)
	{}

	inline size_t
	bytes_per_word() const noexcept
	{
		return bytes_per_word_;
	}

	inline size_t
	bits_per_word() const noexcept
	{
		return bytes_per_word() * 8;
	}

	/* FIXME: use shared_ptr for declaration */
	virtual const jive_record_memlayout *
	map_record(const rcd::declaration * decl) = 0;

	virtual const jive_union_memlayout *
	map_union(const struct unn::declaration * decl) = 0;

	virtual const jive_dataitem_memlayout *
	map_bitstring(size_t nbits) = 0;

	virtual const jive_dataitem_memlayout *
	map_address() = 0;

	const jive_dataitem_memlayout *
	map_value_type(const value::type & type);

private:
	size_t bytes_per_word_;
};

}

#endif
