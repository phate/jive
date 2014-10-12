/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_NODE_H
#define JIVE_VSDG_NODE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <utility>

#include <jive/context.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/operators/base.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/tracker.h>

namespace jive {
namespace base {
	class type;
}
class gate;
class node_normal_form;
class input;
class output;
}

typedef struct jive_node jive_node;
typedef struct jive_node_class jive_node_class;

typedef struct jive_tracker_nodestate jive_tracker_nodestate;

typedef jive::operation jive_node_attrs;

enum jive_traversal_nodestate;

struct jive_node_normal_form_class;
struct jive_region;
struct jive_resource_class_count;
struct jive_substitution_map;

class jive_node {
public:
	virtual ~jive_node() noexcept;
	
	virtual const jive::operation &
	operation() const noexcept = 0;

	inline jive_node * producer(size_t index) const noexcept
	{
		JIVE_DEBUG_ASSERT(index < ninputs);
		return inputs[index]->producer();
	}
	
	const struct jive_node_class * class_;
	
	struct jive_graph * graph;
	
	struct jive_region * region;
	
	int depth_from_root;
	size_t nsuccessors;
	size_t ninputs;
	size_t noperands;
	size_t noutputs;

	std::vector<jive::input*> inputs;
	std::vector<jive::output*> outputs;
	
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

	std::vector<jive_tracker_nodestate*> tracker_slots;
};

extern const jive_node_class JIVE_NODE;

namespace jive {

// Define node representing a specific operation.
template<typename Operation>
class operation_node final : public jive_node {
public:
	virtual ~operation_node() noexcept {}

	// Construct copying the operator.
	operation_node(const Operation & operation)
		: operation_(operation)
	{}

	// Construct moving the operator.
	operation_node(Operation && operation) noexcept
		: operation_(std::move(operation))
	{}

	virtual const Operation &
	operation() const noexcept override
	{
		return operation_;
	}

private:
	Operation operation_;
};

template<typename Operation>
operation_node<Operation> *
create_operation_node(const Operation & operation)
{
	return new operation_node<Operation>(operation);
}

template<typename Operation>
operation_node<Operation> *
create_operation_node(Operation && operation)
{
	return new operation_node<Operation>(std::move(operation));
}

}

struct jive_node_class {
	const struct jive_node_class * parent;
	const char * name;
	
	void (*fini)(jive_node * self);
	
	/** \brief Retrieve node normal form */
	jive::node_normal_form *(*get_default_normal_form)(
		const jive_node_class * cls, jive::node_normal_form * parent, struct jive_graph * graph);
	
	/** \brief Give textual representation of node (for debugging) */
	void (*get_label)(const jive_node * self, struct jive_buffer * buffer);
	
	/** \brief Compare with attribute set (of same node type) */
	bool (*match_attrs)(const jive_node * self, const jive_node_attrs * second);

	void (*check_operands)(const jive_node_class * cls, const jive_node_attrs * attrs,
		size_t noperands, jive::output * const operands[], jive_context * context);
	
	/** \brief Class method, create node with given attributes */
	jive_node * (*create)(struct jive_region * region,
		const jive_node_attrs * attrs,
		size_t noperands, jive::output * const operands[]);
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

JIVE_EXPORTED_INLINE void
jive_node_get_label(const jive_node * self, jive_buffer * buffer)
{
	std::string s = self->operation().debug_string();
	jive_buffer_putstr(buffer, s.c_str());
}

JIVE_EXPORTED_INLINE const jive_node_attrs *
jive_node_get_attrs(const jive_node * self)
{
	return &self->operation();
}

JIVE_EXPORTED_INLINE bool
jive_node_match_attrs(const jive_node * self, const jive_node_attrs * other)
{
	return self->operation() == *other;
}

JIVE_EXPORTED_INLINE jive_node *
jive_node_create(
	const jive_node_class * class_,
	const jive::operation & op,
	jive_region * region,
	size_t noperands,
	jive::output * const operands[])
{
	return op.create_node(region, noperands, operands);
}

JIVE_EXPORTED_INLINE void
jive_node_check_operands(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive::output * const operands[], jive_context * context)
{
}

struct jive_node *
jive_node_copy(
	const jive_node * self, struct jive_region * region, jive::output * operands[]);

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
jive_node_copy_substitute(
	const jive_node * self, struct jive_region * target, struct jive_substitution_map * substitution);

/**
	\brief Check if an edge may be added to the node
	\param self Target node
	\param origin Origin of edge
	
	Check whether an edge to @c self as target node may be added,
	originating at port @c origin.
*/
bool
jive_node_valid_edge(const jive_node * self, const jive::output * origin);

jive::input *
jive_node_add_input(jive_node * self, const jive::base::type * type, jive::output * origin);

jive::output *
jive_node_add_output(jive_node * self, const jive::base::type * type);

jive::output *
jive_node_add_constrained_output(jive_node * self, const struct jive_resource_class * rescls);

jive::input *
jive_node_add_constrained_input(
	jive_node * self, const struct jive_resource_class * rescls, jive::output * initial_operand);

jive::input *
jive_node_gate_input(jive_node * self, jive::gate * gate, jive::output * initial_operand);

JIVE_EXPORTED_INLINE jive::input *
jive_node_get_gate_input(const jive_node * self, const jive::gate * gate)
{
	for (size_t n = 0; n < self->ninputs; n++) {
		if (self->inputs[n]->gate == gate) {
			return self->inputs[n];
		}
	}
	return nullptr;
}

JIVE_EXPORTED_INLINE jive::input *
jive_node_get_gate_input(const jive_node * self, const char * name)
{
	for (size_t n = 0; n < self->ninputs; n++) {
		jive::input * i = self->inputs[n];
		if (i->gate && strcmp(i->gate->name, name) == 0) {
			return i;
		}
	}
	return nullptr;
}

JIVE_EXPORTED_INLINE jive::output *
jive_node_get_gate_output(const jive_node * self, const jive::gate * gate)
{
	for (size_t n = 0; n < self->noutputs; n++) {
		if (self->outputs[n]->gate == gate) {
			return self->outputs[n];
		}
	}
	return nullptr;
}

JIVE_EXPORTED_INLINE jive::output *
jive_node_get_gate_output(const jive_node * self, const char * name)
{
	for (size_t n = 0; n < self->noutputs; n++) {
		jive::output * o = self->outputs[n];
		if (o->gate && strcmp(o->gate->name, name) == 0) {
			return o;
		}
	}
	return nullptr;
}

jive::output *
jive_node_gate_output(jive_node * self, jive::gate * gate);

jive::input *
jive_node_input(const struct jive_node * self, size_t index);

jive::output *
jive_node_output(const struct jive_node * self, size_t index);

void
jive_node_get_use_count_input(
	const jive_node * self,
	struct jive_resource_class_count * use_count, struct jive_context * context);

void
jive_node_get_use_count_output(
	const jive_node * self,
	struct jive_resource_class_count * use_count, struct jive_context * context);

void
jive_node_destroy(jive_node * self);

jive_node *
jive_node_cse(
	jive_region * region,
	const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive::output * const operands[]);

/* normal forms */

extern const jive_node_normal_form_class JIVE_NODE_NORMAL_FORM;

void
jive_node_create_normalized(const jive_node_class * class_, struct jive_graph * graph,
	const jive_node_attrs * attrs, size_t noperands, jive::output * const operands[],
	jive::output * results[]);

/**
	\brief Attempt to find existing or create new node
	
	\param nf Normal form of node class
	\param region Region to create node in
	\param attrs Attributes of node
	\param noperands Number of operands
	\param operands Input operands of node
*/

jive_node *
jive_node_cse_create(
	const jive::node_normal_form * nf,
	struct jive_region * region,
	const jive_node_attrs * attrs,
	size_t noperands,
	jive::output * const operands[]);

bool
jive_node_normalize(struct jive_node * self);

/* tracking support */

jive_tracker_nodestate *
jive_node_get_tracker_state_slow(jive_node * self, jive_tracker_slot slot);

JIVE_EXPORTED_INLINE jive_tracker_nodestate *
jive_node_get_tracker_state(jive_node * self, jive_tracker_slot slot)
{
	jive_tracker_nodestate * nodestate;
	if (slot.index < self->tracker_slots.size()) {
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
jive_region_innermost(size_t noperands, jive::output * const operands[])
{
	jive_region * region = operands[noperands - 1]->node()->region;
	for (size_t n = noperands - 1; n; --n) {
		jive_node * node = operands[n-1]->node();
		if (node->region->depth > region->depth)
			region = node->region;
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
	JIVE_DEBUG_ASSERT(self->bottom->outputs[0]->single_user());
	return self->bottom->outputs[0]->users.first->node;
}

#endif
