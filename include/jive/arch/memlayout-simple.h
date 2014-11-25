/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_MEMLAYOUT_SIMPLE_H
#define JIVE_ARCH_MEMLAYOUT_SIMPLE_H

#include <jive/arch/memlayout.h>

#include <unordered_map>

/* caching support */

typedef struct jive_memlayout_mapper_cached jive_memlayout_mapper_cached;

struct jive_memlayout_mapper_cached {
	jive_memlayout_mapper base;
	std::unordered_map<const jive::rcd::declaration*, jive_record_memlayout> record_map;
	std::unordered_map<const jive::unn::declaration*, jive_union_memlayout> union_map;
	std::unordered_map<size_t, jive_dataitem_memlayout> bitstring_map;
};

void
jive_memlayout_mapper_cached_fini_(jive_memlayout_mapper_cached * self);

struct jive_record_memlayout *
jive_memlayout_mapper_cached_map_record_(jive_memlayout_mapper_cached * self,
	const jive::rcd::declaration * decl);

struct jive_union_memlayout *
jive_memlayout_mapper_cached_map_union_(jive_memlayout_mapper_cached * self,
	const jive::unn::declaration * decl);

struct jive_dataitem_memlayout *
jive_memlayout_mapper_cached_map_bitstring_(jive_memlayout_mapper_cached * self, size_t nbits);

struct jive_record_memlayout *
jive_memlayout_mapper_cached_add_record_(jive_memlayout_mapper_cached * self,
	const jive::rcd::declaration * decl);

struct jive_union_memlayout *
jive_memlayout_mapper_cached_add_union_(jive_memlayout_mapper_cached * self,
	const jive::unn::declaration * decl);

/* simplistic implementation, for simple use cases & testing */

/* FIXME: endianness */
typedef struct jive_memlayout_mapper_simple jive_memlayout_mapper_simple;

struct jive_memlayout_mapper_simple {
	jive_memlayout_mapper_cached base;
	size_t bits_per_word;
	size_t bytes_per_word;
	jive_dataitem_memlayout address_layout;
};

void
jive_memlayout_mapper_simple_init(jive_memlayout_mapper_simple * self, size_t bits_per_word);

void
jive_memlayout_mapper_simple_fini(jive_memlayout_mapper_simple * self);

#endif
