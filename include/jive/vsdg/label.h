/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_LABEL_H
#define JIVE_VSDG_LABEL_H

#include <stdbool.h>
#include <stdint.h>

#include <jive/util/hash.h>
#include <jive/vsdg/node.h>

struct jive_context;
struct jive_graph;
struct jive_node;
struct jive_output;
struct jive_region;

struct jive_seq_graph;
struct jive_seq_point;

typedef uint64_t jive_offset;

typedef struct jive_label_class jive_label_class;
typedef struct jive_label jive_label;
typedef struct jive_label_internal_class jive_label_internal_class;
typedef struct jive_label_internal jive_label_internal;
typedef struct jive_label_node jive_label_node;
typedef struct jive_label_region jive_label_region;
typedef struct jive_label_external jive_label_external;
typedef struct jive_address jive_address;

struct jive_address {
	jive_offset offset;
	jive_stdsectionid section;
};

JIVE_EXPORTED_INLINE void
jive_address_init(jive_address * self, jive_stdsectionid section, jive_offset offset)
{
	self->offset = offset;
	self->section = section;
}

struct jive_label_class {
	const jive_label_class * parent;
	void (*fini)(jive_label * self);
};

extern const jive_label_class JIVE_LABEL;

typedef enum {
	jive_label_flags_none = 0,
	jive_label_flags_global = 1, /* whether label is supposed to be "globally visible" */
	jive_label_flags_external = 2, /* whether label must be resolved outside the current graph */
} jive_label_flags;

struct jive_label {
	const jive_label_class * class_;
	
	jive_label_flags flags;
};

JIVE_EXPORTED_INLINE void
jive_label_fini(jive_label * self)
{
	self->class_->fini(self);
}

JIVE_EXPORTED_INLINE bool
jive_label_isinstance(const jive_label * self, const jive_label_class * class_)
{
	const jive_label_class * cls = self->class_;
	while (cls) {
		if (cls == class_)
			return true;
		cls = cls->parent;
	}
	return false;
}

struct jive_label_internal_class {
	jive_label_class base;
};

extern const jive_label_internal_class JIVE_LABEL_INTERNAL_;
#define JIVE_LABEL_INTERNAL (JIVE_LABEL_INTERNAL_.base)

struct jive_label_internal {
	jive_label base;
	struct jive_graph * graph;
	struct {
		jive_label_internal * prev;
		jive_label_internal * next;
	} graph_label_list;
	char * asmname;
};

struct jive_label_node {
	jive_label_internal base;
	struct jive_node * node;
};

struct jive_label_region {
	jive_label_internal base;
	struct jive_region * region;
};

struct jive_label_external {
	jive_label base;
	struct jive_context * context;
	char * asmname;
};

/**
	\brief Special label marking position of "current" instruction
*/
extern const jive_label jive_label_current;

extern const jive_label_class JIVE_LABEL_CURRENT;

/**
	\brief Special label marking offset from frame pointer
*/
extern const jive_label jive_label_fpoffset;

extern const jive_label_class JIVE_LABEL_FPOFFSET;

/**
	\brief Special label marking offset from stack pointer
*/
extern const jive_label jive_label_spoffset;

extern const jive_label_class JIVE_LABEL_SPOFFSET;

/**
	\brief Label where node is sequenced
*/
jive_label *
jive_label_node_create(struct jive_node * node);

/**
	\brief Label where node is sequenced
	
	Node not specified until later -- dangerous function,
	only to be used during serialization.
*/
jive_label_node *
jive_label_node_create_dangling(struct jive_graph * graph);

extern const jive_label_internal_class JIVE_LABEL_NODE_;
#define JIVE_LABEL_NODE (JIVE_LABEL_NODE_.base)

/**
	\brief Label where start of region is sequenced
	
	Region not specified until later -- dangerous function,
	only to be used during serialization.
*/
jive_label_region *
jive_label_region_start_create_dangling(struct jive_graph * graph);

/**
	\brief Label where start of region is sequenced
*/
jive_label *
jive_label_region_start_create(struct jive_region * region);

/**
	\brief Exported label where start of region is sequenced
*/
jive_label *
jive_label_region_start_create_exported(struct jive_region * region, const char * name);

extern const jive_label_internal_class JIVE_LABEL_REGION_START_;
#define JIVE_LABEL_REGION_START (JIVE_LABEL_REGION_START_.base)

/**
	\brief Label where end of region is sequenced
	
	Region not specified until later -- dangerous function,
	only to be used during serialization.
*/
jive_label_region *
jive_label_region_end_create_dangling(struct jive_graph * graph);

/**
	\brief Label where end of region is sequenced
*/
jive_label *
jive_label_region_end_create(struct jive_region * region);

extern const jive_label_internal_class JIVE_LABEL_REGION_END_;
#define JIVE_LABEL_REGION_END (JIVE_LABEL_REGION_END_.base)

/**
	\brief Exported label where end of region is sequenced
*/
jive_label *
jive_label_region_end_create_exported(struct jive_region * region, const char * name);

extern const jive_label_class JIVE_LABEL_EXTERNAL;

/**
	\brief Initialize label external
*/
void
jive_label_external_init(
	jive_label_external * self,
	struct jive_context * context,
	const char * name);

/**
	\brief Finalize label external
*/
void
jive_label_external_fini(jive_label_external * self);

#endif
