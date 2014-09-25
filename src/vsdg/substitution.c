/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/context.h>

#include <jive/vsdg/substitution.h>

struct jive_output_substitution {
	const jive::output * original;
	jive::output * substitute;
	
	struct {
		jive_output_substitution * prev;
		jive_output_substitution * next;
	} hash_chain;
};

struct jive_region_substitution {
	const struct jive_region * original;
	struct jive_region * substitute;
	
	struct {
		jive_region_substitution * prev;
		jive_region_substitution * next;
	} hash_chain;
};

struct jive_gate_substitution {
	const jive::gate * original;
	jive::gate * substitute;
	
	struct {
		jive_gate_substitution * prev;
		jive_gate_substitution * next;
	} hash_chain;
};

JIVE_DEFINE_HASH_TYPE(jive_output_substitution_hash, jive_output_substitution,
	const jive::output *, original, hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_region_substitution_hash, jive_region_substitution, const struct jive_region *, original, hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_gate_substitution_hash, jive_gate_substitution, const jive::gate *,
	original, hash_chain);

jive_substitution_map *
jive_substitution_map_create(struct jive_context * context)
{
	jive_substitution_map * map = new jive_substitution_map;
	jive_output_substitution_hash_init(&map->output_hash, context);
	jive_region_substitution_hash_init(&map->region_hash, context);
	jive_gate_substitution_hash_init(&map->gate_hash, context);
	return map;
}

void
jive_substitution_map_add_output(jive_substitution_map * self, const jive::output * original,
	jive::output * substitute)
{
	jive_output_substitution * subst = jive_output_substitution_hash_lookup(&self->output_hash, original);
	if (!subst) {
		subst = jive_context_malloc(self->output_hash.context, sizeof(*subst));
		subst->original = original;
		subst->substitute = substitute;
		jive_output_substitution_hash_insert(&self->output_hash, subst);
	} else {
		subst->substitute = substitute;
	}
}

jive::output *
jive_substitution_map_lookup_output(const jive_substitution_map * self,
	const jive::output * original)
{
	jive_output_substitution * subst = jive_output_substitution_hash_lookup(&self->output_hash, original);
	if (!subst)
		return NULL;
	else
		return subst->substitute;
}

void
jive_substitution_map_add_region(jive_substitution_map * self, const struct jive_region * original, struct jive_region * substitute)
{
	jive_region_substitution * subst = jive_region_substitution_hash_lookup(&self->region_hash, original);
	if (!subst) {
		subst = jive_context_malloc(self->region_hash.context, sizeof(*subst));
		subst->original = original;
		subst->substitute = substitute;
		jive_region_substitution_hash_insert(&self->region_hash, subst);
	} else {
		subst->substitute = substitute;
	}
}

struct jive_region *
jive_substitution_map_lookup_region(const jive_substitution_map * self, const struct jive_region * original)
{
	jive_region_substitution * subst = jive_region_substitution_hash_lookup(&self->region_hash, original);
	if (!subst)
		return NULL;
	else
		return subst->substitute;
}

void
jive_substitution_map_add_gate(jive_substitution_map * self, const jive::gate * original,
	jive::gate * substitute)
{
	jive_gate_substitution * subst = jive_gate_substitution_hash_lookup(&self->gate_hash, original);
	if (!subst) {
		subst = jive_context_malloc(self->gate_hash.context, sizeof(*subst));
		subst->original = original;
		subst->substitute = substitute;
		jive_gate_substitution_hash_insert(&self->gate_hash, subst);
	} else {
		subst->substitute = substitute;
	}
}

jive::gate *
jive_substitution_map_lookup_gate(const jive_substitution_map * self, const jive::gate * original)
{
	jive_gate_substitution * subst = jive_gate_substitution_hash_lookup(&self->gate_hash, original);
	if (!subst)
		return NULL;
	else
		return subst->substitute;
}

void
jive_substitution_map_destroy(jive_substitution_map * self)
{
	jive_context * context = self->output_hash.context;
	
	struct jive_output_substitution_hash_iterator i;
	i = jive_output_substitution_hash_begin(&self->output_hash);
	while(i.entry) {
		jive_output_substitution * subst = i.entry;
		jive_output_substitution_hash_iterator_next(&i);
		jive_output_substitution_hash_remove(&self->output_hash, subst);
		jive_context_free(context, subst);
	}
	
	struct jive_region_substitution_hash_iterator j;
	j = jive_region_substitution_hash_begin(&self->region_hash);
	while(j.entry) {
		jive_region_substitution * subst = j.entry;
		jive_region_substitution_hash_iterator_next(&j);
		jive_region_substitution_hash_remove(&self->region_hash, subst);
		jive_context_free(context, subst);
	}
	
	struct jive_gate_substitution_hash_iterator k;
	k = jive_gate_substitution_hash_begin(&self->gate_hash);
	while(k.entry) {
		jive_gate_substitution * subst = k.entry;
		jive_gate_substitution_hash_iterator_next(&k);
		jive_gate_substitution_hash_remove(&self->gate_hash, subst);
		jive_context_free(context, subst);
	}
	
	jive_output_substitution_hash_fini(&self->output_hash);
	jive_region_substitution_hash_fini(&self->region_hash);
	jive_gate_substitution_hash_fini(&self->gate_hash);
	delete self;
}
