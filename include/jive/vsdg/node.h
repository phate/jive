#ifndef JIVE_VSDG_NODE_H
#define JIVE_VSDG_NODE_H

#include <stdlib.h>
#include <stdbool.h>

#include <jive/vsdg/crossings.h>

typedef struct jive_node jive_node;
typedef struct jive_node_class jive_node_class;

struct jive_graph;
struct jive_input;
struct jive_type;
struct jive_output;
struct jive_region;
struct jive_traverser_nodestate;

struct jive_node {
	const struct jive_node_class * class_;
	
	struct jive_graph * graph;
	
	struct jive_region * region;
	
	int depth_from_root;
	size_t nsuccessors;
	size_t ninputs;
	size_t noperands;
	size_t noutputs;
	
	struct jive_input ** inputs;
	struct jive_output ** outputs;
	
	void * shape_location; /* TODO: data type */
	
	jive_xpoint_hash resource_crossings;
	void * active_before_resources; /* TODO: data type */
	void * active_after_resources; /* TODO: data type */
	void * use_count_before; /* TODO: data type */
	void * use_count_after; /* TODO: data type */
	
	struct {
		jive_node * prev;
		jive_node * next;
	} region_nodes_list;
	
	struct {
		jive_node * prev;
		jive_node * next;
	} graph_top_list;
	
	struct {
		jive_node * prev;
		jive_node * next;
	} graph_bottom_list;
	
	size_t ntraverser_slots;
	struct jive_traverser_nodestate ** traverser_slots;
};

typedef enum jive_node_type_flags {
	jive_node_class_associative = 1,
	jive_node_class_commutative = 2
} jive_node_class_flags;

struct jive_node_class {
	const struct jive_node_class * parent;
	const char * name;
	jive_node_class_flags flags;
	
	void (*fini)(jive_node * self);
	
	/** \brief Give textual representation of node (for debugging) */
	char * (*get_label)(const jive_node * self);
	
	/** \brief Copy node, using specified inputs as operands */
	jive_node * (*copy)(const jive_node * self,
		struct jive_region * region,
		struct jive_output * operands[]);
		
	/** \brief Test for equivalence with another node */
	bool (*equiv)(const jive_node * self, const jive_node * other);

#if 0
	/** \brief Invalidate any computed state depending on inputs (i.e. value range) */
	void (*invalidate_inputs)(jive_node * self);
	
	/** \brief Recompute any auxiliary state of outputs */
	void (*revalidate_outputs)(jive_node * self);
#endif
};

static inline bool
jive_node_isinstance(const jive_node * self, const jive_node_class * class_)
{
	const jive_node_class * c = self->class_;
	while(c) {
		if (c == class_) return true;
		c = c->parent;
	}
	return false;
}

/* returned string allocated with malloc */
static inline char *
jive_node_get_label(const jive_node * self)
{
	return self->class_->get_label(self);
}

static inline jive_node *
jive_node_copy(const jive_node * self,
	struct jive_region * region,
	struct jive_output * operands[])
{
	return self->class_->copy(self, region, operands);
}

static inline bool
jive_node_equiv(const jive_node * self, const jive_node * other)
{
	return self->class_->equiv(self, other);
}

struct jive_input *
jive_node_add_input(jive_node * self, const struct jive_type * type, struct jive_output * initial_operand);

struct jive_output *
jive_node_add_output(jive_node * self, const struct jive_type * type);

void
jive_node_destroy(jive_node * self);

#endif
