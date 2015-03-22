/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_NODE_H
#define JIVE_VSDG_NODE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <utility>

#include <jive/util/buffer.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/gate-interference.h>
#include <jive/vsdg/operators/base.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/tracker.h>

namespace jive {
namespace base {
	class type;
}

class gate;
class node_normal_form;

/**
        \defgroup jive::input Inputs
        Inputs
        @{
*/

class input final {
public:
	~input() noexcept;

	input(
		struct jive_node * node,
		size_t index,
		jive::output * origin,
		const jive::base::type & type);

public:
	inline const jive::base::type &
	type() const noexcept
	{
		return *type_;
	}

	inline std::string
	debug_string() const;

	inline size_t
	index() const noexcept
	{
		return index_;
	}

	inline struct jive_node *
	node() const noexcept
	{
		return node_;
	}

	/*
		FIXME: Try to merge internal_divert_origin and divert_origin methods.
	*/
	void internal_divert_origin(jive::output * new_origin) noexcept;

	void divert_origin(jive::output * new_origin) noexcept;

	/*
		FIXME: This function is only used two times in src/regalloc/fixup.c. See whether we can
		actually remove it and add a replacement in the register allocator.
	*/
	void swap(input * other) noexcept;

	inline jive::output * origin() const noexcept { return origin_; }

	inline jive_node * producer() const noexcept;

	struct {
		input * prev;
		input * next;
	} output_users_list;

	jive::gate * gate;
	struct {
		input * prev;
		input * next;
	} gate_inputs_list;

	struct jive_ssavar * ssavar;
	struct {
		input * prev;
		input * next;
	} ssavar_input_list;

	struct {
		struct jive_region_hull_entry * first;
		struct jive_region_hull_entry * last;
	} hull;

	const struct jive_resource_class * required_rescls;

private:
	size_t index_;
	jive::output * origin_;
	struct jive_node * node_;

	/*
		FIXME: This attribute is necessary as long as the number of inputs do not coincide with the
		number given by the operation. Once this is fixed, the attribute can be removed and the type
		can be taken from the operation.
	*/
	std::unique_ptr<jive::base::type> type_;
};

}	//jive namespace

struct jive_variable *
jive_input_get_constraint(const jive::input * self);

void
jive_input_unassign_ssavar(jive::input * self);

struct jive_ssavar *
jive_input_auto_assign_variable(jive::input * self);

struct jive_ssavar *
jive_input_auto_merge_variable(jive::input * self);

namespace jive {

/**	@}	*/

/**
        \defgroup jive::output Outputs
        Outputs
        @{
*/

class output final {
public:
	~output() noexcept;

	output(struct jive_node * node, size_t index, const jive::base::type & type);

public:
	const jive::base::type &
	type() const noexcept
	{
		return *type_;
	}

	inline std::string
	debug_string() const;

	inline jive_node * node() const noexcept { return node_; }

	inline bool no_user() const noexcept { return users.first == nullptr; }

	inline bool single_user() const noexcept
		{ return (users.first != nullptr) && (users.first == users.last); }

	size_t index;

	struct {
		jive::input * first;
		jive::input * last;
	} users;

	jive::gate * gate;
	struct {
		jive::output * prev;
		jive::output * next;
	} gate_outputs_list;

	struct jive_ssavar * ssavar;

	struct {
		struct jive_ssavar * first;
		struct jive_ssavar * last;
	} originating_ssavars;

	const struct jive_resource_class * required_rescls;
private:
	jive_node * node_;

	/*
		FIXME: This attribute is necessary as long as the number of inputs do not coincide with the
		number given by the operation. Once this is fixed, the attribute can be removed and the type
		can be taken from the operation.
	*/
	std::unique_ptr<jive::base::type> type_;
};

}	//jive namespace

struct jive_variable *
jive_output_get_constraint(const jive::output * self);

void
jive_output_replace(jive::output * self, jive::output * other);

struct jive_ssavar *
jive_output_auto_assign_variable(jive::output * self);

struct jive_ssavar *
jive_output_auto_merge_variable(jive::output * self);

namespace jive {

/**	@}	*/

/**
        \defgroup jive::gate Gates
        Gates
        @{
*/

class gate final {
public:
	~gate() noexcept;

	gate(struct jive_graph * graph, const char name[], const jive::base::type & type);

public:
	const jive::base::type &
	type() const noexcept
	{
		return *type_;
	}

	inline std::string
	debug_string() const
	{
		return name;
	}

	jive::input * create_input(jive_node * node, size_t index, jive::output * origin);

	jive::output * create_output(jive_node * node, size_t index);

	struct jive_graph * graph;
	struct {
		jive::gate * prev;
		jive::gate * next;
	} graph_gate_list;

	std::string name;

	struct {
		jive::input * first;
		jive::input * last;
	} inputs;

	struct {
		jive::output * first;
		jive::output * last;
	} outputs;

	bool may_spill;
	jive_gate_interference_hash interference;

	struct jive_variable * variable;
	struct {
		jive::gate * prev;
		jive::gate * next;
	} variable_gate_list;

	const struct jive_resource_class * required_rescls;

private:
	/*
		FIXME: This attribute is necessary as long as the number of inputs do not coincide with the
		number given by the operation. Once this is fixed, the attribute can be removed and the type
		can be taken from the operation.
	*/
	std::unique_ptr<jive::base::type> type_;
};

}	//jive namespace

struct jive_variable *
jive_gate_get_constraint(jive::gate * self);

size_t
jive_gate_interferes_with(const jive::gate * self, const jive::gate * other);

void
jive_gate_merge(jive::gate * self, jive::gate * other);

void
jive_gate_split(jive::gate * self);

void
jive_gate_auto_merge_variable(jive::gate * self);

/**	@}	*/

/**	@}	*/

inline jive_node *
jive::input::producer() const noexcept
{
	return origin_->node();
}

inline std::string
jive::input::debug_string() const
{
	if (gate)
		return gate->debug_string();

	std::string str;
	str += index_;
	return str;
}

inline std::string
jive::output::debug_string() const
{
	if (gate)
		return gate->debug_string();

	std::string str;
	str += index;
	return str;
}

typedef struct jive_node jive_node;

typedef struct jive_tracker_nodestate jive_tracker_nodestate;

struct jive_region;
struct jive_resource_class_count;
struct jive_substitution_map;

class jive_node final {
public:
	~jive_node();

	inline jive_node(std::unique_ptr<jive::operation> op)
		: operation_(std::move(op))
	{
	}

	inline const jive::operation &
	operation() const noexcept
	{
		return *operation_;
	}

	inline jive_node * producer(size_t index) const noexcept
	{
		JIVE_DEBUG_ASSERT(index < ninputs);
		return inputs[index]->producer();
	}
	
	struct jive_graph * graph;
	
	struct jive_region * region;
	
	size_t depth_from_root;
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

private:
	std::unique_ptr<jive::operation> operation_;
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
	const jive_node * self,
	struct jive_region * target,
	jive::substitution_map & substitution);

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
		if (i->gate && i->gate->name == name)
			return i;
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
		if (o->gate && o->gate->name == name)
			return o;
	}
	return nullptr;
}

jive::output *
jive_node_gate_output(jive_node * self, jive::gate * gate);

jive::input *
jive_node_input(const struct jive_node * self, size_t index);

jive::output *
jive_node_output(const struct jive_node * self, size_t index);

JIVE_EXPORTED_INLINE jive_region *
jive_node_anchored_region(const jive_node * self, size_t index)
{
	jive_region * region = self->inputs[index]->origin()->node()->region;
	/* the given region can only be different if the identified input
	 * is of "anchor" type, so this implicitly checks the type */
	JIVE_DEBUG_ASSERT(self->region != region);
	return region;
}

void
jive_node_get_use_count_input(
	const jive_node * self,
	struct jive_resource_class_count * use_count);

void
jive_node_get_use_count_output(
	const jive_node * self,
	struct jive_resource_class_count * use_count);

void
jive_node_destroy(jive_node * self);

JIVE_EXPORTED_INLINE std::vector<jive::output *>
jive_node_arguments(jive_node * self)
{
	std::vector<jive::output *> arguments;
	for (size_t n = 0; n < self->noperands; ++n) {
		arguments.push_back(self->inputs[n]->origin());
	}
	return arguments;
}

jive_node *
jive_node_cse(
	jive_region * region,
	const jive::operation & op,
	const std::vector<jive::output *> & arguments);

/* normal forms */

std::vector<jive::output *>
jive_node_create_normalized(
	jive_graph * graph,
	const jive::operation & op,
	const std::vector<jive::output *> & arguments);

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
	jive_region * region,
	const jive::operation & op,
	const std::vector<jive::output *> & arguments);

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
	return self->bottom->outputs[0]->users.first->node();
}

#endif
