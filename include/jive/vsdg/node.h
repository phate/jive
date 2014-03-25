/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_NODE_H
#define JIVE_VSDG_NODE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <jive/context.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/tracker.h>

typedef struct jive_node jive_node;
typedef struct jive_node_attrs jive_node_attrs;
typedef struct jive_node_class jive_node_class;

typedef struct jive_node_normal_form jive_node_normal_form;
typedef struct jive_node_normal_form_class jive_node_normal_form_class;
typedef struct jive_tracker_nodestate jive_tracker_nodestate;

struct jive_input;
struct jive_type;
struct jive_output;
struct jive_gate;
struct jive_region;
enum jive_traversal_nodestate;
struct jive_resource_class_count;
struct jive_substitution_map;

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
	} region_top_node_list;
	
	struct {
		jive_node * prev;
		jive_node * next;
	} graph_bottom_list;
	
	size_t ntraverser_slots;
	enum jive_traversal_nodestate ** traverser_slots;
	
	size_t ntracker_slots;
	jive_tracker_nodestate ** tracker_slots;
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
	jive_node_normal_form *(*get_default_normal_form)(const jive_node_class * cls, jive_node_normal_form * parent, struct jive_graph * graph);
	
	/** \brief Give textual representation of node (for debugging) */
	void (*get_label)(const jive_node * self, struct jive_buffer * buffer);
	
	/** \brief Retrieve attributes of node */
	const jive_node_attrs * (*get_attrs)(const jive_node * self);
	
	/** \brief Compare with attribute set (of same node type) */
	bool (*match_attrs)(const jive_node * self, const jive_node_attrs * second);

	void (*check_operands)(const jive_node_class * cls, const jive_node_attrs * attrs,
		size_t noperands, jive_output * const operands[], jive_context * context);
	
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

struct jive_tracker_nodestate {
	jive_node * node;
	size_t cookie;
	size_t state;
	intptr_t tag;
	struct {
		jive_tracker_nodestate * prev;
		jive_tracker_nodestate * next;
	} state_node_list;
};

JIVE_EXPORTED_INLINE bool
jive_node_isinstance(const jive_node * self, const jive_node_class * class_)
{
	const jive_node_class * c = self->class_;
	while(c) {
		if (c == class_) return true;
		c = c->parent;
	}
	return false;
}

JIVE_EXPORTED_INLINE void
jive_node_get_label(const jive_node * self, struct jive_buffer * buffer)
{
	self->class_->get_label(self, buffer);
}

JIVE_EXPORTED_INLINE const jive_node_attrs *
jive_node_get_attrs(const jive_node * self)
{
	return self->class_->get_attrs(self);
}

JIVE_EXPORTED_INLINE bool
jive_node_match_attrs(const jive_node * self, const jive_node_attrs * other)
{
	return self->class_->match_attrs(self, other);
}

JIVE_EXPORTED_INLINE void
jive_node_check_operands(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context)
{
	return cls->check_operands(cls, attrs, noperands, operands, context);
}

JIVE_EXPORTED_INLINE const struct jive_resource_class *
jive_node_get_aux_rescls(const jive_node * self)
{
	return self->class_->get_aux_rescls(self);
}

struct jive_node *
jive_node_copy(const jive_node * self, struct jive_region * region, struct jive_output * operands[]);

/**
	\brief Copy a node with substitutions
	\param self Node to be copied
	\param target Target region to create node in
	\param substitution Operand and gate substitutions
	\return Copied node
	
	Create a new node that is semantically equivalent to an
	existing node. The newly created node will use the same
	operands as the existing node unless there is a substitution
	registered for a particular operand.
	
	The given substitution map will be update so that all
	outputs of the original node will be substituted by
	corresponding outputs of the newly created node in
	subsequent \ref jive_node_copy_substitute operations.
*/
jive_node *
jive_node_copy_substitute(const jive_node * self, struct jive_region * target, struct jive_substitution_map * substitution);

JIVE_EXPORTED_INLINE void
jive_node_reserve(jive_node * self)
{
	self->reserved ++;
}

JIVE_EXPORTED_INLINE void
jive_node_unreserve(jive_node * self)
{
	self->reserved --;
}

/**
	\brief Check if an edge may be added to the node
	\param self Target node
	\param origin Origin of edge
	
	Check whether an edge to @c self as target node may be added,
	originating at port @c origin.
*/
bool
jive_node_valid_edge(const jive_node * self, const struct jive_output * origin);

struct jive_input *
jive_node_add_input(jive_node * self, const struct jive_type * type, struct jive_output * initial_operand);

struct jive_output *
jive_node_add_output(jive_node * self, const struct jive_type * type);

struct jive_output *
jive_node_add_constrained_output(jive_node * self, const struct jive_resource_class * rescls);

struct jive_input *
jive_node_add_constrained_input(jive_node * self, const struct jive_resource_class * rescls, struct jive_output * initial_operand);

struct jive_input *
jive_node_gate_input(jive_node * self, struct jive_gate * gate, struct jive_output * initial_operand);

JIVE_EXPORTED_INLINE struct jive_input *
jive_node_get_gate_input(const jive_node * self, const struct jive_gate * gate)
{
	size_t n;
	for(n = 0; n < self->ninputs; n++)
		if (self->inputs[n]->gate == gate) return self->inputs[n];
	return 0;
}

JIVE_EXPORTED_INLINE struct jive_output *
jive_node_get_gate_output(const jive_node * self, const struct jive_gate * gate)
{
	size_t n;
	for(n = 0; n < self->noutputs; n++)
		if (self->outputs[n]->gate == gate) return self->outputs[n];
	return 0;
}

struct jive_output *
jive_node_gate_output(jive_node * self, struct jive_gate * gate);

struct jive_input *
jive_node_input(const struct jive_node * self, size_t index);

struct jive_output *
jive_node_output(const struct jive_node * self, size_t index);

void
jive_node_get_use_count_input(const jive_node * self, struct jive_resource_class_count * use_count, struct jive_context * context);

void
jive_node_get_use_count_output(const jive_node * self, struct jive_resource_class_count * use_count, struct jive_context * context);

void
jive_node_destroy(jive_node * self);

jive_node *
jive_node_cse(
	jive_region * region,
	const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[]);

/* normal forms */

struct jive_node_normal_form_class {
	const jive_node_normal_form_class * parent;
	void (*fini)(jive_node_normal_form * self);
	/* return true, if normalized already */
	bool (*normalize_node)(const jive_node_normal_form * self, jive_node * node);
	/* return true, if normalized already */
	bool (*operands_are_normalized)(const jive_node_normal_form * self, size_t noperands, jive_output * const operands[], const jive_node_attrs * attrs);
	void (*normalized_create)(const jive_node_normal_form * self, struct jive_graph * graph,
		const jive_node_attrs * attrs, size_t noperands, jive_output * const operands[],
		jive_output * results[]);
	void (*set_mutable)(jive_node_normal_form * self, bool enable);
	void (*set_cse)(jive_node_normal_form * self, bool enable);
};

extern const jive_node_normal_form_class JIVE_NODE_NORMAL_FORM;

struct jive_node_normal_form {
	const jive_node_normal_form_class * class_;
	const jive_node_class * node_class;
	
	jive_node_normal_form * parent;
	struct jive_graph * graph;
	struct {
		jive_node_normal_form * first;
		jive_node_normal_form * last;
	} subclasses;
	struct {
		jive_node_normal_form * prev;
		jive_node_normal_form * next;
	} normal_form_subclass_list;
	struct {
		jive_node_normal_form * prev;
		jive_node_normal_form * next;
	} hash_chain;
	
	bool enable_mutable;
	bool enable_cse;
};

JIVE_EXPORTED_INLINE bool
jive_node_normal_form_isinstance(const jive_node_normal_form * self,
	const jive_node_normal_form_class * class_)
{
	const jive_node_normal_form_class * cls = self->class_;
	while (cls) {
		if (cls == class_)
			return true;
		cls = cls->parent;
	}
	return false;
}

JIVE_EXPORTED_INLINE void
jive_node_normal_form_fini(jive_node_normal_form * self)
{
	self->class_->fini(self);
}

JIVE_EXPORTED_INLINE bool
jive_node_normal_form_normalize_node(const jive_node_normal_form * self, jive_node * node)
{
	return self->class_->normalize_node(self, node);
}

JIVE_EXPORTED_INLINE bool
jive_node_normal_form_operands_are_normalized(const jive_node_normal_form * self,
	size_t noperands, jive_output * const operands[],
	const jive_node_attrs * attrs)
{
	return self->class_->operands_are_normalized(self, noperands, operands, attrs);
}

JIVE_EXPORTED_INLINE void
jive_node_normal_form_normalized_create(const jive_node_normal_form * self,
	struct jive_graph * graph, const jive_node_attrs * attrs, size_t noperands,
	jive_output * const operands[], jive_output * results[])
{
	return self->class_->normalized_create(self, graph, attrs, noperands, operands, results);
}

JIVE_EXPORTED_INLINE void
jive_node_normal_form_set_mutable(jive_node_normal_form * self, bool enable)
{
	return self->class_->set_mutable(self, enable);
}

JIVE_EXPORTED_INLINE bool
jive_node_normal_form_get_mutable(jive_node_normal_form * self)
{
	return self->enable_mutable;
}

JIVE_EXPORTED_INLINE void
jive_node_normal_form_set_cse(jive_node_normal_form * self, bool enable)
{
	return self->class_->set_cse(self, enable);
}

JIVE_EXPORTED_INLINE bool
jive_node_normal_form_get_cse(jive_node_normal_form * self)
{
	return self->enable_cse;
}

void
jive_node_create_normalized(const jive_node_class * class_, struct jive_graph * graph,
	const jive_node_attrs * attrs, size_t noperands, jive_output * const operands[],
	jive_output * results[]);

/**
	\brief Attempt to find existing or create new node
	
	\param nf Normal form of node class
	\param region Region to create node in
	\param attrs Attributes of node
	\param noperands Number of operands
	\param operands Input operands of node
*/

jive_node *
jive_node_cse_create(const jive_node_normal_form * nf, struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[]);

bool
jive_node_normalize(struct jive_node * self);

/* tracking support */

jive_tracker_nodestate *
jive_node_get_tracker_state_slow(jive_node * self, jive_tracker_slot slot);

JIVE_EXPORTED_INLINE jive_tracker_nodestate *
jive_node_get_tracker_state(jive_node * self, jive_tracker_slot slot)
{
	jive_tracker_nodestate * nodestate;
	if (slot.index < self->ntracker_slots) {
		nodestate = self->tracker_slots[slot.index];
		if (nodestate->cookie != slot.cookie) {
			nodestate->state = jive_tracker_nodestate_none;
			nodestate->cookie = slot.cookie;
			nodestate->tag = 0;
		}
		return nodestate;
	}
	return jive_node_get_tracker_state_slow(self, slot);
}

/**
	\brief Test whether node can be moved to next outer region
*/
bool
jive_node_can_move_outward(const jive_node * self);

/**
	\brief Move node to outer region
*/
void
jive_node_move_outward(jive_node * self);

bool
jive_node_can_move_inward(const jive_node * self);

void
jive_node_move_inward(jive_node * self);

bool
jive_node_depends_on_region(const jive_node * self, const struct jive_region * region);

struct jive_region *
jive_node_next_inner_region(const jive_node * self);

void
jive_node_move(jive_node * self, struct jive_region * new_region);

/* methods pertaining to jive_region that require definition of jive_node
need to live here to avoid cyclic header dependency */

JIVE_EXPORTED_INLINE bool
jive_region_contains_node(const jive_region * self, const jive_node * node)
{
	const jive_region * tmp = node->region;
	while(tmp->depth >= self->depth) {
		if (tmp == self) return true;
		tmp = tmp->parent;
		if (!tmp) break;
	}
	return false;
}

/** \brief Determine innermost of multiple (possibly) nested regions from operand list */
JIVE_EXPORTED_INLINE jive_region *
jive_region_innermost(size_t noperands, jive_output * const operands[])
{
	jive_region * region = operands[0]->node->region;
	size_t n;
	for(n = 1; n < noperands; n++) {
		if (operands[n]->node->region->depth > region->depth)
			region = operands[n]->node->region;
	}
	
	return region;
}

JIVE_EXPORTED_INLINE struct jive_node *
jive_region_get_anchor_node(const struct jive_region * self)
{
	if (self->bottom == NULL)
		return NULL;
	if (self->parent == NULL)
		return NULL;

	JIVE_DEBUG_ASSERT(self->bottom->noutputs == 1);
	JIVE_DEBUG_ASSERT(jive_output_has_single_user(self->bottom->outputs[0]));
	return self->bottom->outputs[0]->users.first->node;
}

#endif
