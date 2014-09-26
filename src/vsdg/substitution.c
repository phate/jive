/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/substitution.h>

jive_substitution_map *
jive_substitution_map_create(struct jive_context * context)
{
	return new jive_substitution_map;
}

void
jive_substitution_map_add_output(jive_substitution_map * self, const jive::output * original,
	jive::output * substitute)
{
	self->output_hash[original] = substitute;
}

jive::output *
jive_substitution_map_lookup_output(const jive_substitution_map * self,
	const jive::output * original)
{
	auto i = self->output_hash.find(original);
	if (i == self->output_hash.end())
		return nullptr;
	else
		return i->second;
}

void
jive_substitution_map_add_region(jive_substitution_map * self, const struct jive_region * original,
	struct jive_region * substitute)
{
	self->region_hash[original] = substitute;
}

struct jive_region *
jive_substitution_map_lookup_region(const jive_substitution_map * self,
	const struct jive_region * original)
{
	auto i = self->region_hash.find(original);
	if (i == self->region_hash.end())
		return nullptr;
	else
		return i->second;
}

void
jive_substitution_map_add_gate(jive_substitution_map * self, const jive::gate * original,
	jive::gate * substitute)
{
	self->gate_hash[original] = substitute;
}

jive::gate *
jive_substitution_map_lookup_gate(const jive_substitution_map * self, const jive::gate * original)
{
	auto i = self->gate_hash.find(original);
	if (i == self->gate_hash.end())
		return nullptr;
	else
		return i->second;
}

void
jive_substitution_map_destroy(jive_substitution_map * self)
{
	delete self;
}
