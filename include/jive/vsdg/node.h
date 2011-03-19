#ifndef JIVE_VSDG_NODE_H
#define JIVE_VSDG_NODE_H

#include <stdlib.h>
#include <stdbool.h>

#include <jive/vsdg/basetype.h>

typedef struct jive_node jive_node;
typedef struct jive_node_attrs jive_node_attrs;
typedef struct jive_node_class jive_node_class;

typedef struct jive_node_normal_form jive_node_normal_form;

struct jive_graph;
struct jive_input;
struct jive_type;
struct jive_output;
struct jive_gate;
struct jive_region;
struct jive_traversal_nodestate;

struct jive_node {
	const struct jive_node_class * class_;
	
	struct jive_graph * graph;
	
	struct jive_region * region;
	
	int depth_from_root;
	size_t nsuccessors;
	size_t ninputs;
	size_t noperands;
	size_t noutputs;
	size_t reserved;
	
	struct jive_input ** inputs;
	struct jive_output ** outputs;
	
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
	struct jive_traversal_nodestate ** traverser_slots;
};

extern const jive_node_class JIVE_NODE;

struct jive_node_attrs {
	/* empty, need override */
};

struct jive_node_class {
	const struct jive_node_class * parent;
	const char * name;
	
	void (*fini)(jive_node * self);
	
	/** \brief Retrieve node normal form */
	const jive_node_normal_form *(*get_default_normal_form)(const jive_node * self);
	
	/** \brief Give textual representation of node (for debugging) */
	char * (*get_label)(const jive_node * self);
	
	/** \brief Retrieve attributes of node */
	const jive_node_attrs * (*get_attrs)(const jive_node * self);
	
	/** \brief Compare with attribute set (of same node type) */
	bool (*match_attrs)(const jive_node * self, const jive_node_attrs * second);
	
	/** \brief Class method, create node with given attributes */
	jive_node * (*create)(struct jive_region * region,
		const jive_node_attrs * attrs,
		size_t noperands, struct jive_output * const operands[]);
	
	/**
		\brief Get auxiliary resource class
		
		Covers one corner case for two-operand architectures,
		returns NULL otherwise.
	*/
	const struct jive_resource_class * (*get_aux_rescls)(const jive_node * self);
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

static inline const jive_node_attrs *
jive_node_get_attrs(const jive_node * self)
{
	return self->class_->get_attrs(self);
}

static inline bool
jive_node_match_attrs(const jive_node * self, const jive_node_attrs * other)
{
	return self->class_->match_attrs(self, other);
}

static inline const struct jive_resource_class *
jive_node_get_aux_rescls(const jive_node * self)
{
	return self->class_->get_aux_rescls(self);
}

static inline jive_node *
jive_node_copy(const jive_node * self,
	struct jive_region * region,
	struct jive_output * operands[])
{
	return self->class_->create(region, jive_node_get_attrs(self), self->noperands, operands);
}

static inline void
jive_node_reserve(jive_node * self)
{
	self->reserved ++;
}

static inline void
jive_node_unreserve(jive_node * self)
{
	self->reserved --;
}

struct jive_input *
jive_node_add_input(jive_node * self, const struct jive_type * type, struct jive_output * initial_operand);

struct jive_output *
jive_node_add_output(jive_node * self, const struct jive_type * type);

struct jive_input *
jive_node_gate_input(jive_node * self, struct jive_gate * gate, struct jive_output * initial_operand);

static inline struct jive_input *
jive_node_get_gate_input(const jive_node * self, const struct jive_gate * gate)
{
	size_t n;
	for(n = 0; n < self->ninputs; n++)
		if (self->inputs[n]->gate == gate) return self->inputs[n];
	return 0;
}

static inline struct jive_output *
jive_node_get_gate_output(const jive_node * self, const struct jive_gate * gate)
{
	size_t n;
	for(n = 0; n < self->noutputs; n++)
		if (self->outputs[n]->gate == gate) return self->outputs[n];
	return 0;
}

struct jive_output *
jive_node_gate_output(jive_node * self, struct jive_gate * gate);

void
jive_node_destroy(jive_node * self);

#endif
