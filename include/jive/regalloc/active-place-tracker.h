#ifndef JIVE_REGALLOC_ACTIVE_PLACE_TRACKER_H
#define JIVE_REGALLOC_ACTIVE_PLACE_TRACKER_H

#include <jive/vsdg/regcls-count.h>
#include <jive/util/hash.h>

struct jive_output;
struct jive_resource;

typedef struct jive_active_place jive_active_place;
typedef struct jive_conflict jive_conflict;
typedef struct jive_active_place_tracker jive_active_place_tracker;

struct jive_active_place {
	struct jive_output * origin;
	struct jive_resource * resource;
	int locked;
	int priority;
	
	struct {
		jive_active_place * prev;
		jive_active_place * next;
	} hash_by_resource;
	
	struct {
		jive_active_place * prev;
		jive_active_place * next;
	} hash_by_origin;
};

typedef enum {
	jive_conflict_none = 0,
	jive_conflict_register_class,
	jive_conflict_resource_name
} jive_conflict_type;

struct jive_conflict {
	jive_conflict_type type;
	union {
		const struct jive_regcls * regcls;
		jive_active_place * place;
	};
};

JIVE_DECLARE_HASH_TYPE(jive_active_resource_map, jive_active_place, struct jive_resource *, resource, hash_by_resource);
JIVE_DECLARE_HASH_TYPE(jive_active_origin_map, jive_active_place, struct jive_output *, origin, hash_by_origin);

struct jive_active_place_tracker {
	jive_context * context;
	struct jive_active_resource_map resource_map;
	struct jive_active_origin_map origin_map;
	jive_regcls_count use_count;
};

#endif
