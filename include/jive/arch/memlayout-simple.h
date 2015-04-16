/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_MEMLAYOUT_SIMPLE_H
#define JIVE_ARCH_MEMLAYOUT_SIMPLE_H

#include <jive/arch/memlayout.h>

#include <unordered_map>

namespace jive {

/* simplistic implementation, for simple use cases & testing */

/* FIXME: endianness */
class memlayout_mapper_simple final : public memlayout_mapper {
public:
	virtual
	~memlayout_mapper_simple();

	memlayout_mapper_simple(size_t bytes_per_word);

	virtual const jive_record_memlayout *
	map_record(const rcd::declaration * decl) override;

	virtual const jive_union_memlayout *
	map_union(const struct unn::declaration * decl) override;

	virtual const jive_dataitem_memlayout *
	map_bitstring(size_t nbits) override;

	virtual const jive_dataitem_memlayout *
	map_address()	override;

private:
	jive_record_memlayout &
	add_record(const rcd::declaration * decl);

	jive_union_memlayout &
	add_union(const unn::declaration * decl);

	jive_dataitem_memlayout address_layout_;
	std::unordered_map<size_t, jive_dataitem_memlayout> bitstring_map_;
	std::unordered_map<const jive::unn::declaration*, jive_union_memlayout> union_map_;
	std::unordered_map<const jive::rcd::declaration*, jive_record_memlayout> record_map_;
};

}

#endif
