/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/substitution.h>

jive_substitution_map *
jive_substitution_map_create()
{
	return new jive_substitution_map;
}

void
jive_substitution_map_add_output(jive_substitution_map * self, const jive::output * original,
	jive::output * substitute)
{
	self->map.insert(original, substitute);
}

jive::output *
jive_substitution_map_lookup_output(const jive_substitution_map * self,
	const jive::output * original)
{
	return self->map.lookup(original);
}

void
jive_substitution_map_add_region(jive_substitution_map * self, const struct jive_region * original,
	struct jive_region * substitute)
{
	self->map.insert(original, substitute);
}

struct jive_region *
jive_substitution_map_lookup_region(const jive_substitution_map * self,
	const struct jive_region * original)
{
	return self->map.lookup(original);
}

void
jive_substitution_map_add_gate(jive_substitution_map * self, const jive::gate * original,
	jive::gate * substitute)
{
	self->map.insert(original, substitute);
}

jive::gate *
jive_substitution_map_lookup_gate(const jive_substitution_map * self, const jive::gate * original)
{
	return self->map.lookup(original);
}

void
jive_substitution_map_destroy(jive_substitution_map * self)
{
	delete self;
}
