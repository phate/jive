/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_MEMLAYOUT_SIMPLE_H
#define JIVE_ARCH_MEMLAYOUT_SIMPLE_H

#include <jive/context.h>
#include <jive/arch/memlayout.h>
#include <jive/util/hash.h>
#include <jive/util/rangemap.h>

/* caching support */

typedef struct jive_memlayout_mapper_cached jive_memlayout_mapper_cached;

typedef struct jive_memlayout_record_entry jive_memlayout_record_entry;
typedef struct jive_memlayout_union_entry jive_memlayout_union_entry;
typedef struct jive_memlayout_record_hash jive_memlayout_record_hash;
typedef struct jive_memlayout_union_hash jive_memlayout_union_hash;
typedef struct jive_memlayout_bitstring_map jive_memlayout_bitstring_map;

struct jive_memlayout_record_entry {
	const struct jive_record_declaration * decl;
	jive_record_memlayout layout;
	struct {
		jive_memlayout_record_entry * prev, * next;
	} hash_chain;
};

struct jive_memlayout_union_entry {
	const struct jive_union_declaration * decl;
	jive_union_memlayout layout;
	struct {
		jive_memlayout_union_entry * prev, * next;
	} hash_chain;
};

JIVE_DECLARE_HASH_TYPE(jive_memlayout_record_hash, jive_memlayout_record_entry, const struct jive_record_declaration *, decl, hash_chain);
JIVE_DECLARE_HASH_TYPE(jive_memlayout_union_hash, jive_memlayout_union_entry, const struct jive_union_declaration *, decl, hash_chain);
JIVE_DECLARE_RANGEMAP_TYPE(jive_memlayout_bitstring_map, jive_dataitem_memlayout *, NULL);
struct jive_memlayout_mapper_cached {
	jive_memlayout_mapper base;
	jive_context * context;
	jive_memlayout_record_hash record_hash;
	jive_memlayout_union_hash union_hash;
	jive_memlayout_bitstring_map bitstring_map;
};

void
jive_memlayout_mapper_cached_init_(jive_memlayout_mapper_cached * self, jive_context * context);

void
jive_memlayout_mapper_cached_fini_(jive_memlayout_mapper_cached * self);

struct jive_record_memlayout *
jive_memlayout_mapper_cached_map_record_(jive_memlayout_mapper_cached * self, const struct jive_record_declaration * decl);

struct jive_union_memlayout *
jive_memlayout_mapper_cached_map_union_(jive_memlayout_mapper_cached * self, const struct jive_union_declaration * decl);

struct jive_dataitem_memlayout **
jive_memlayout_mapper_cached_map_bitstring_(jive_memlayout_mapper_cached * self, size_t nbits);

struct jive_record_memlayout *
jive_memlayout_mapper_cached_add_record_(jive_memlayout_mapper_cached * self, const struct jive_record_declaration * decl);

struct jive_union_memlayout *
jive_memlayout_mapper_cached_add_union_(jive_memlayout_mapper_cached * self, const struct jive_union_declaration * decl);

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
jive_memlayout_mapper_simple_init(jive_memlayout_mapper_simple * self, jive_context * context, size_t bits_per_word);

void
jive_memlayout_mapper_simple_fini(jive_memlayout_mapper_simple * self);

#endif
