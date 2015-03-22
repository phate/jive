/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_SUBSTITUTION_H
#define JIVE_VSDG_SUBSTITUTION_H

#include <jive/common.h>

#include <unordered_map>

namespace jive {
	class gate;
	class output;
}

struct jive_graph;
struct jive_region;

namespace jive {

class substitution_map final {
public:
	inline jive::output *
	lookup(const jive::output * original) const noexcept
	{
		auto i = output_map_.find(original);
		return i != output_map_.end() ? i->second : nullptr;
	}

	inline jive_region *
	lookup(const jive_region * original) const noexcept
	{
		auto i = region_map_.find(original);
		return i != region_map_.end() ? i->second : nullptr;
	}

	inline jive::gate *
	lookup(const jive::gate * original) const noexcept
	{
		auto i = gate_map_.find(original);
		return i != gate_map_.end() ? i->second : nullptr;
	}

	inline void
	insert(const jive::output * original, jive::output * substitute)
	{
		output_map_[original] = substitute;
	}

	inline void
	insert(const jive_region * original, jive_region * substitute)
	{
		region_map_[original] = substitute;
	}
	inline void
	insert(const jive::gate * original, jive::gate * substitute)
	{
		gate_map_[original] = substitute;
	}

private:
	std::unordered_map<const jive::gate*, jive::gate*> gate_map_;
	std::unordered_map<const jive_region*, jive_region*> region_map_;
	std::unordered_map<const jive::output*, jive::output*> output_map_;
};

}

typedef struct jive_substitution_map jive_substitution_map;

struct jive_substitution_map {
	jive::substitution_map map;
	std::unordered_map<const jive::output*, jive::output*> output_hash;
	std::unordered_map<const jive_region*, jive_region*> region_hash;
	std::unordered_map<const jive::gate*, jive::gate*> gate_hash;
};

jive_substitution_map *
jive_substitution_map_create();

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
