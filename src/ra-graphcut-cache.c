#include "ra-graphcut-cache.h"
#include <jive/internal/debug.h>

struct _jive_graphcut_state_cache {
	jive_graph * graph;
	unsigned int nbuckets, nentries;
	jive_graphcut_state **buckets;
	
	jive_graphcut_state *unused;
};

/* use a simple linear hash on the addresses of the nodes involved,
this is sufficiently unique */
static unsigned int
jive_graphcut_state_hash(const jive_graphcut_state * state)
{
	size_t n;
	unsigned int v=0;
	for(n=0; n<state->instr.nitems; n++)
		v = 5 * v + (long)state->instr.items[n];
	for(n=0; n<state->regs.nitems; n++)
		v = 5 * v + (long)state->regs.items[n];
	return v;
}

static bool
jive_graphcut_state_equals(const jive_graphcut_state * state1, const jive_graphcut_state * state2)
{
	size_t n;
	if (state1->instr.nitems != state2->instr.nitems) return false;
	if (state1->regs.nitems != state2->regs.nitems) return false;
	
	for(n=0; n<state1->instr.nitems; n++)
		if (state1->instr.items[n] != state2->instr.items[n])
			return false;
	for(n=0; n<state2->regs.nitems; n++)
		if (state1->regs.items[n] != state2->regs.items[n])
			return false;
	return true;
}



jive_graphcut_state_cache *
jive_graphcut_state_cache_create(jive_graph * graph)
{
	jive_graphcut_state_cache *cache;
	cache = jive_malloc(graph, sizeof(*cache));
	cache->graph = graph;
	cache->nbuckets = 3;
	cache->nentries = 0;
	cache->buckets = jive_malloc(graph, cache->nbuckets * sizeof(cache->buckets[0]));
	memset(cache->buckets, 0, cache->nbuckets * sizeof(cache->buckets[0]));
	cache->unused = 0;
	
	return cache;
}

const jive_graphcut_state *
jive_graphcut_state_search(
	jive_graphcut_state_cache * cache,
	const jive_graphcut_state * reference)
{
	unsigned int hash = jive_graphcut_state_hash(reference);
	hash = hash % cache->nbuckets;
	jive_graphcut_state * state = cache->buckets[hash];
	while(state) {
		if (jive_graphcut_state_equals(state, reference))
			return state;
		state = state->hash_next;
	}
	return 0;
}

void
jive_graphcut_state_insert(
	jive_graphcut_state_cache * cache,
	jive_graphcut_state *state)
{
	/* rehash if hash table too full */
	if (cache->nentries >= cache->nbuckets) {
		unsigned new_nbuckets = cache->nbuckets*2+1, n;
		jive_graphcut_state ** new_buckets;
		new_buckets = jive_malloc(cache->graph, new_nbuckets * sizeof(*new_buckets));
		memset(new_buckets, 0, new_nbuckets * sizeof(*new_buckets));
		for(n=0; n<cache->nbuckets; n++) {
			while(cache->buckets[n]) {
				jive_graphcut_state *tmp = cache->buckets[n];
				cache->buckets[n] = tmp->hash_next;
				unsigned int hash = jive_graphcut_state_hash(tmp) % new_nbuckets;
				tmp->hash_next = new_buckets[hash];
				new_buckets[hash]=tmp;
			}
		}
		cache->buckets = new_buckets;
		cache->nbuckets = new_nbuckets;
	}
	unsigned int hash = jive_graphcut_state_hash(state);
	hash = hash % cache->nbuckets;
	state->hash_next = cache->buckets[hash];
	cache->buckets[hash] = state;
	cache->nentries++;
}

void
jive_graphcut_state_cache_clear(jive_graphcut_state_cache * cache)
{
	unsigned int n;
	for(n=0; n<cache->nbuckets; n++) {
		while(cache->buckets[n]) {
			jive_graphcut_state * state=cache->buckets[n];
			cache->buckets[n] = state->hash_next;
			jive_graphcut_state_destroy(cache, state);
			cache->nentries--;
		}
	}
	DEBUG_ASSERT(cache->nentries==0);
}

void
jive_graphcut_state_destroy(
	jive_graphcut_state_cache *cache,
	jive_graphcut_state *state)
{
	state->hash_next = cache->unused;
	cache->unused = state;
}

static jive_graphcut_state *
jive_graphcut_state_allocate(jive_graphcut_state_cache * cache)
{
	jive_graphcut_state * state;
	if (cache->unused) {
		state = cache->unused;
		cache->unused = state->hash_next;
		jive_instruction_list_clear(&state->instr);
		jive_register_list_clear(&state->regs);
		memset(state->budget, 0, sizeof(int)*8);
		state->hash_next = 0;
		return state;
	}
	state = jive_malloc(cache->graph, sizeof(*state));
	jive_instruction_list_init(&state->instr);
	jive_register_list_init(&state->regs);
	memset(state->budget, 0, sizeof(int)*8);
	state->hash_next = 0;
	return state;
}

/* currently, ordering of registers and instructions matters;
it might be worthwhile to investigate sorting to make states
that differ only in ordering equivalent, but I doubt it */
jive_graphcut_state *
jive_graphcut_state_create(
	jive_graphcut_state_cache * cache,
	jive_graphcut *cut)
{
	jive_graphcut_state * state = jive_graphcut_state_allocate(cache);
	size_t n;
	
	for(n=0; n<cut->registers.nitems; n++)
		jive_register_list_append(&state->regs,  cut->registers.items[n]);
	for(n=0; n<cut->available.nitems; n++)
		jive_instruction_list_append(&state->instr, cut->available.items[n]);
	
	for(n=0; n<MAX_REGISTER_CLASSES; n++) state->budget[n]=cut->regs_budget[n];
	
	return state;
}

