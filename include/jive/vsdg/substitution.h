/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_SUBSTITUTION_H
#define JIVE_VSDG_SUBSTITUTION_H

#include <unordered_map>

namespace jive {
	class gate;
	class output;
}

struct jive_context;
struct jive_graph;
struct jive_region;

typedef struct jive_substitution_map jive_substitution_map;

struct jive_substitution_map {
	std::unordered_map<const jive::output*, jive::output*> output_hash;
	std::unordered_map<const jive_region*, jive_region*> region_hash;
	std::unordered_map<const jive::gate*, jive::gate*> gate_hash;
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
jive_substitution_map_add_region(jive_substitution_map * self, const struct jive_region * original,
	struct jive_region * substitute);

struct jive_region *
jive_substitution_map_lookup_region(const jive_substitution_map * self,
	const struct jive_region * original);

void
jive_substitution_map_add_gate(jive_substitution_map * self, const jive::gate * original,
	jive::gate * substitute);

jive::gate *
jive_substitution_map_lookup_gate(const jive_substitution_map * self, const jive::gate * original);

void
jive_substitution_map_destroy(jive_substitution_map * self);

#endif
