/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_MEMLAYOUT_SIMPLE_H
#define JIVE_ARCH_MEMLAYOUT_SIMPLE_H

#include <jive/arch/memlayout.h>

#include <unordered_map>

/* simplistic implementation, for simple use cases & testing */

/* FIXME: endianness */
typedef struct jive_memlayout_mapper_simple jive_memlayout_mapper_simple;

struct jive_memlayout_mapper_simple {
	jive_memlayout_mapper base;
	size_t bits_per_word;
	size_t bytes_per_word;
	jive_dataitem_memlayout address_layout;
	std::unordered_map<const jive::rcd::declaration*, jive_record_memlayout> record_map;
	std::unordered_map<const jive::unn::declaration*, jive_union_memlayout> union_map;
	std::unordered_map<size_t, jive_dataitem_memlayout> bitstring_map;
};

void
jive_memlayout_mapper_simple_init(jive_memlayout_mapper_simple * self, size_t bits_per_word);

void
jive_memlayout_mapper_simple_fini(jive_memlayout_mapper_simple * self);

#endif
