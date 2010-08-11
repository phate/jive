#ifndef JIVE_VSDG_CROSSINGS_H
#define JIVE_VSDG_CROSSINGS_H

#include <jive/context.h>
#include <jive/util/hash.h>

/*
	The data structures below keep track of node/resource
	interactions. Each resource can appear in multiple roles
	with respect to each node:
	
	- "active before": resource is used as input or passed through
	- "crossed": resource is passed through
	- "active after": resurce is uned as output or passed through
	
	Each resource can appear in each role multiple times.
	
	For tracking these role counts, a hash (that maps resource
	-> interaction point) is stored per node. Additionally
	the interaction points are linked per resource.
*/

struct jive_node;
struct jive_resource;

typedef struct jive_node_resource_interaction_hash_bucket jive_node_resource_interaction_hash_bucket;
typedef struct jive_node_interaction jive_node_interaction;
typedef struct jive_resource_interaction jive_resource_interaction;

/* interaction points of single node with resources, hashed by resource */

JIVE_DECLARE_HASH_TYPE(jive_resource_interaction, struct jive_node_resource_interaction, struct jive_resource *, resource, same_node_list);

/* interaction points of single resource with nodes */

struct jive_node_interaction {
	struct jive_node_resource_interaction * first;
	struct jive_node_resource_interaction * last;
};

#endif
