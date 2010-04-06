#ifndef JIVE_INTERNAL_RA_GRAPHCUT_CACHE
#define JIVE_INTERNAL_RA_GRAPHCUT_CACHE

#include "ra-common.h"

/**
	\brief Examined graphcut state
*/
typedef struct _jive_graphcut_state jive_graphcut_state;
struct _jive_graphcut_state {
	/* instructions elegible for selection in this state */
	jive_instruction_list instr;
	/* active registers in this state */
	jive_register_list regs;
	/* register allocator budget in this state */
	int budget[8];
	/* next element hashing to the same value */
	jive_graphcut_state * hash_next;
};

typedef struct _jive_graphcut_state_cache jive_graphcut_state_cache;

/** \brief Create cache of states that have already been examined */
jive_graphcut_state_cache *
jive_graphcut_state_cache_create(jive_graph * graph);

/**
	\brief Empty the state cache
 
 	Must be called when the graph has been modified
 	as the examined states have become invalid
*/
void
jive_graphcut_state_cache_clear(jive_graphcut_state_cache * cache);

/**
	\brief Create a state object corresponding to the given graph cut
	
	The state object contains a description of the available
	instructions, registers as well as register budget.
*/
jive_graphcut_state *
jive_graphcut_state_create(
	jive_graphcut_state_cache * cache,
	jive_graphcut *cut);

/**
	\brief Search the state cache
	
	Search the set of recorded states if we find an identical one.
	Returns the existing state if a match was found, or NULL
	if no match was found.
	
	A state is considered matching if it contains the same set of
	active registers as well as schedulable instructions, but
	disregards the set register budget.
*/
const jive_graphcut_state *
jive_graphcut_state_search(
	jive_graphcut_state_cache * cache,
	const jive_graphcut_state * reference);

/**
	\brief Destroy a state object
	
	Destroys the state object. The associated memory is not freed,
	the object is returned to the cache for future use. Caller
	may only use this function on objects created through
	\ref graphcut_state_create but that have not yet been
	placed into the state cache by \ref graphcut_state_insert.
*/
void
jive_graphcut_state_destroy(
	jive_graphcut_state_cache * cache,
	jive_graphcut_state * state);

/**
	\brief Insert the given state into the cache
	
	Inserts the given state into the cache for later lookup.
	Caller has to make sure that no matching state exists
	(see \ref graphcut_state_search) before calling this
	function.
*/
void
graphcut_state_insert(
	jive_graphcut_state_cache * cache,
	jive_graphcut_state * state);

#endif
