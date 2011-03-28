#ifndef JIVE_REGALLOC_XPOINT_H
#define JIVE_REGALLOC_XPOINT_H

#include <jive/util/hash.h>

typedef struct jive_xpoint jive_xpoint;
typedef struct jive_tpoint jive_tpoint;

struct jive_shaped_node;
struct jive_shaped_region;
struct jive_shaped_ssavar;

struct jive_xpoint {
	struct jive_shaped_node * shaped_node;
	struct jive_shaped_ssavar * shaped_ssavar;
	
	size_t before_count;
	size_t cross_count;
	size_t after_count;
	
	struct {
		struct jive_xpoint * prev;
		struct jive_xpoint * next;
	} node_hash_chain;
	
	struct {
		struct jive_xpoint * prev;
		struct jive_xpoint * next;
	} ssavar_hash_chain;
};

JIVE_DECLARE_HASH_TYPE(jive_node_xpoint_hash, jive_xpoint, struct jive_shaped_node *, shaped_node, node_hash_chain);
JIVE_DECLARE_HASH_TYPE(jive_ssavar_xpoint_hash, jive_xpoint, struct jive_shaped_ssavar *, shaped_ssavar, ssavar_hash_chain);

typedef struct jive_node_xpoint_hash jive_node_xpoint_hash;
typedef struct jive_ssavar_xpoint_hash jive_ssavar_xpoint_hash;

struct jive_tpoint {
	struct jive_shaped_region * shaped_region;
	struct jive_shaped_ssavar * shaped_ssavar;
	
	size_t count;
	
	struct {
		struct jive_tpoint * prev;
		struct jive_tpoint * next;
	} region_hash_chain;
	
	struct {
		struct jive_tpoint * prev;
		struct jive_tpoint * next;
	} ssavar_hash_chain;
};

JIVE_DECLARE_HASH_TYPE(jive_region_tpoint_hash, jive_tpoint, struct jive_shaped_region *, shaped_region, region_hash_chain);
JIVE_DECLARE_HASH_TYPE(jive_ssavar_tpoint_hash, jive_tpoint, struct jive_shaped_ssavar *, shaped_ssavar, ssavar_hash_chain);

typedef struct jive_region_tpoint_hash jive_region_tpoint_hash;
typedef struct jive_ssavar_tpoint_hash jive_ssavar_tpoint_hash;

#endif
