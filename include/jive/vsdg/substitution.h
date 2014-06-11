/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_SUBSTITUTION_H
#define JIVE_VSDG_SUBSTITUTION_H

#include <jive/util/hash.h>

namespace jive {
	class output;
}

struct jive_context;
struct jive_graph;
struct jive_region;
struct jive_gate;

typedef struct jive_substitution_map jive_substitution_map;
typedef struct jive_output_substitution jive_output_substitution;
typedef struct jive_region_substitution jive_region_substitution;
typedef struct jive_gate_substitution jive_gate_substitution;

typedef struct jive_output_substitution_hash jive_output_substitution_hash;
typedef struct jive_region_substitution_hash jive_region_substitution_hash;
typedef struct jive_gate_substitution_hash jive_gate_substitution_hash;
JIVE_DECLARE_HASH_TYPE(jive_output_substitution_hash, jive_output_substitution, const jive::output *,
	original, hash_chain);
JIVE_DECLARE_HASH_TYPE(jive_region_substitution_hash, jive_region_substitution, const struct jive_region *, original, hash_chain);
JIVE_DECLARE_HASH_TYPE(jive_gate_substitution_hash, jive_gate_substitution, const struct jive_gate *, original, hash_chain);

struct jive_substitution_map {
	jive_output_substitution_hash output_hash;
	jive_region_substitution_hash region_hash;
	jive_gate_substitution_hash gate_hash;
};

jive_substitution_map *
jive_substitution_map_create(struct jive_context * context);

void
jive_substitution_map_add_output(jive_substitution_map * self, const jive::output * original,
	jive::output * substitute);

struct jive::output *
jive_substitution_map_lookup_output(const jive_substitution_map * self,
	const jive::output * original);

void
jive_substitution_map_add_region(jive_substitution_map * self, const struct jive_region * original, struct jive_region * substitute);

struct jive_region *
jive_substitution_map_lookup_region(const jive_substitution_map * self, const struct jive_region * original);

void
jive_substitution_map_add_gate(jive_substitution_map * self, const struct jive_gate * original, struct jive_gate * substitute);

struct jive_gate *
jive_substitution_map_lookup_gate(const jive_substitution_map * self, const struct jive_gate * original);

void
jive_substitution_map_destroy(jive_substitution_map * self);

#endif
