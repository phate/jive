#ifndef JIVE_REGALLOC_XPOINT_H
#define JIVE_REGALLOC_XPOINT_H

#include <jive/util/hash.h>

typedef struct jive_nodevar_xpoint jive_nodevar_xpoint;
typedef struct jive_cutvar_xpoint jive_cutvar_xpoint;

struct jive_shaped_node;
struct jive_shaped_region;
struct jive_shaped_ssavar;

struct jive_nodevar_xpoint {
	struct jive_shaped_node * shaped_node;
	struct jive_shaped_ssavar * shaped_ssavar;
	
	size_t before_count;
	size_t cross_count;
	size_t after_count;
	
	struct {
		struct jive_nodevar_xpoint * prev;
		struct jive_nodevar_xpoint * next;
	} node_hash_chain;
	
	struct {
		struct jive_nodevar_xpoint * prev;
		struct jive_nodevar_xpoint * next;
	} ssavar_hash_chain;
};

JIVE_DECLARE_HASH_TYPE(jive_nodevar_xpoint_hash_bynode, jive_nodevar_xpoint, struct jive_shaped_node *, shaped_node, node_hash_chain);
JIVE_DECLARE_HASH_TYPE(jive_nodevar_xpoint_hash_byssavar, jive_nodevar_xpoint, struct jive_shaped_ssavar *, shaped_ssavar, ssavar_hash_chain);

typedef struct jive_nodevar_xpoint_hash_bynode jive_nodevar_xpoint_hash_bynode;
typedef struct jive_nodevar_xpoint_hash_byssavar jive_nodevar_xpoint_hash_byssavar;

struct jive_cutvar_xpoint {
	struct jive_shaped_region * shaped_region;
	struct jive_shaped_ssavar * shaped_ssavar;
	
	size_t count;
	
	struct {
		struct jive_cutvar_xpoint * prev;
		struct jive_cutvar_xpoint * next;
	} region_hash_chain;
	
	struct {
		struct jive_cutvar_xpoint * prev;
		struct jive_cutvar_xpoint * next;
	} ssavar_hash_chain;
};

JIVE_DECLARE_HASH_TYPE(jive_cutvar_xpoint_hash_byregion, jive_cutvar_xpoint, struct jive_shaped_region *, shaped_region, region_hash_chain);
JIVE_DECLARE_HASH_TYPE(jive_cutvar_xpoint_hash_byssavar, jive_cutvar_xpoint, struct jive_shaped_ssavar *, shaped_ssavar, ssavar_hash_chain);

typedef struct jive_cutvar_xpoint_hash_byregion jive_cutvar_xpoint_hash_byregion;
typedef struct jive_cutvar_xpoint_hash_byssavar jive_cutvar_xpoint_hash_byssavar;

#endif
