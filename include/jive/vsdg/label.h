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

typedef uint64_t jive_addr;

typedef struct jive_label_class jive_label_class;
typedef struct jive_label jive_label;
typedef struct jive_label_internal_class jive_label_internal_class;
typedef struct jive_label_internal jive_label_internal;
typedef struct jive_label_node jive_label_node;
typedef struct jive_label_region jive_label_region;
typedef struct jive_label_external jive_label_external;

struct jive_label_class {
	const jive_label_class * parent;
	void (*fini)(jive_label * self);
	jive_addr (*get_address)(const jive_label * self, const struct jive_seq_point * for_point);
	const char * (*get_asmname)(const jive_label * self);
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

JIVE_EXPORTED_INLINE jive_addr
jive_label_get_address(const jive_label * self, const struct jive_seq_point * for_point)
{
	return self->class_->get_address(self, for_point);
}

JIVE_EXPORTED_INLINE const char *
jive_label_get_asmname(const jive_label * self)
{
	return self->class_->get_asmname(self);
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
	struct jive_seq_point * (*get_attach_point)(const jive_label_internal * self, const struct jive_seq_graph * seq_graph);
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

JIVE_EXPORTED_INLINE jive_addr
jive_label_internal_get_address(const jive_label_internal * self, const struct jive_seq_point * for_point)
{
	return jive_label_get_address(&self->base, for_point);
}

JIVE_EXPORTED_INLINE const char *
jive_label_internal_get_asmname(const jive_label_internal * self)
{
	return jive_label_get_asmname(&self->base);
}

JIVE_EXPORTED_INLINE struct jive_seq_point *
jive_label_internal_get_attach_node(const jive_label_internal * self, const struct jive_seq_graph * seq_graph)
{
	const jive_label_internal_class * cls = (const jive_label_internal_class *) self->base.class_;
	return cls->get_attach_point(self, seq_graph);
}

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
	jive_addr address;
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

extern const jive_label_internal_class JIVE_LABEL_NODE_;
#define JIVE_LABEL_NODE (JIVE_LABEL_NODE_.base)

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
	\brief Initialize label external to the graph from which it is referenced
*/
void
jive_label_external_init(jive_label_external * self, struct jive_context * context, const char * name, jive_addr address);

/**
	\brief Finalize label external to the graph from which it is referenced
*/
void
jive_label_external_fini(jive_label_external * self);

#endif
