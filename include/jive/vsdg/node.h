/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_NODE_H
#define JIVE_VSDG_NODE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <unordered_set>
#include <utility>

#include <jive/util/buffer.h>
#include <jive/util/intrusive-list.h>
#include <jive/util/strfmt.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/gate-interference.h>
#include <jive/vsdg/operators/base.h>
#include <jive/vsdg/resource.h>
#include <jive/vsdg/tracker.h>

struct jive_ssavar;
struct jive_variable;

namespace jive {
namespace base {
	class type;
}

class gate;
class node_normal_form;
class oport;
class substitution_map;

class iport {
public:
	virtual
	~iport() noexcept;

	iport(size_t index);

	iport(const iport &) = delete;

	iport(const iport &&) = delete;

	iport &
	operator=(const iport &) = delete;

	iport &
	operator=(const iport &&) = delete;

	inline size_t
	index() const noexcept
	{
		return index_;
	}

	virtual const jive::base::type &
	type() const noexcept = 0;

	virtual std::string
	debug_string() const;

	inline void
	set_index(size_t index) noexcept
	{
		index_ = index;
	}

private:
	size_t index_;
};

/**
        \defgroup jive::input Inputs
        Inputs
        @{
*/

class input final : public iport {
	friend jive::output;

public:
	virtual
	~input() noexcept;

	input(
		struct jive_node * node,
		size_t index,
		jive::oport * origin,
		const jive::base::type & type);

	input(
		jive_node * node,
		size_t index,
		jive::oport * origin,
		jive::gate * gate);

public:
	virtual const jive::base::type &
	type() const noexcept override;

	virtual std::string
	debug_string() const override;

	inline struct jive_node *
	node() const noexcept
	{
		return node_;
	}

	inline jive::gate *
	gate() const noexcept
	{
		return gate_;
	}

	/*
		FIXME: Try to merge internal_divert_origin and divert_origin methods.
	*/
	void internal_divert_origin(jive::output * new_origin) noexcept;

	void divert_origin(jive::output * new_origin) noexcept;

	inline jive::output *
	origin() const noexcept;

	inline jive_node * producer() const noexcept;

	struct jive_variable *
	constraint();

	void
	unassign_ssavar();

	struct jive_ssavar *
	auto_merge_variable();

	struct {
		input * prev;
		input * next;
	} gate_inputs_list;

	struct jive_ssavar * ssavar;
	struct {
		input * prev;
		input * next;
	} ssavar_input_list;

	const struct jive_resource_class * required_rescls;

private:
	jive::gate * gate_;
	jive::oport * origin_;
	struct jive_node * node_;

	/*
		FIXME: This attribute is necessary as long as the number of inputs do not coincide with the
		number given by the operation. Once this is fixed, the attribute can be removed and the type
		can be taken from the operation.
	*/
	std::unique_ptr<jive::base::type> type_;
};


class oport {
public:
	virtual
	~oport() noexcept;

	oport(size_t index);

	oport(const oport &) = delete;

	oport(const oport &&) = delete;

	oport &
	operator=(const oport &) = delete;

	oport &
	operator=(const oport &&) = delete;

	inline size_t
	index() const noexcept
	{
		return index_;
	}

	inline void
	set_index(size_t index) noexcept
	{
		index_ = index;
	}

	virtual const jive::base::type &
	type() const noexcept = 0;

	virtual std::string
	debug_string() const;

private:
	size_t index_;
};

/**	@}	*/

/**
        \defgroup jive::output Outputs
        Outputs
        @{
*/

class output final : public oport {
	friend jive::input;

public:
	virtual
	~output() noexcept;

	output(struct jive_node * node, size_t index, const jive::base::type & type);

public:
	virtual const jive::base::type &
	type() const noexcept override;

	virtual std::string
	debug_string() const override;

	inline jive_node * node() const noexcept { return node_; }

	inline bool
	no_user() const noexcept
	{
		return users.empty();
	}

	inline bool
	single_user() const noexcept
	{
		return users.size() == 1;
	}

	void
	replace(jive::output * other) noexcept;

	std::unordered_set<jive::input*> users;

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

	void
	add_user(jive::input * user) noexcept;

	void
	remove_user(jive::input * user) noexcept;

	jive_node * node_;

	/*
		FIXME: This attribute is necessary as long as the number of inputs do not coincide with the
		number given by the operation. Once this is fixed, the attribute can be removed and the type
		can be taken from the operation.
	*/
	std::unique_ptr<jive::base::type> type_;
};


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
		return name();
	}

	inline struct jive_graph *
	graph() const noexcept
	{
		return graph_;
	}

	inline const std::string &
	name() const noexcept
	{
		return name_;
	}

	void
	split();

	struct {
		jive::gate * prev;
		jive::gate * next;
	} graph_gate_list;

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
	std::string name_;
	struct jive_graph * graph_;

	/*
		FIXME: This attribute is necessary as long as the number of inputs do not coincide with the
		number given by the operation. Once this is fixed, the attribute can be removed and the type
		can be taken from the operation.
	*/
	std::unique_ptr<jive::base::type> type_;
};

}	//jive namespace

/**	@}	*/

/**	@}	*/

inline jive_node *
jive::input::producer() const noexcept
{
	return origin()->node();
}

inline jive::output *
jive::input::origin() const noexcept
{
	/* FIXME: the dynamic_cast is going to be removed again once we introduced region arguments */
	return dynamic_cast<jive::output*>(origin_);
}

typedef struct jive_node jive_node;

typedef struct jive_tracker_nodestate jive_tracker_nodestate;

struct jive_region;
struct jive_resource_class_count;
struct jive_substitution_map;

class jive_node final {
public:
	~jive_node();

	jive_node(std::unique_ptr<jive::operation> op, jive_region * region,
		const std::vector<jive::output*> & operands);

	inline const jive::operation &
	operation() const noexcept
	{
		return *operation_;
	}

	inline jive_node * producer(size_t index) const noexcept
	{
		return input(index)->producer();
	}

	inline bool
	has_successors() const noexcept
	{
		for (auto output : outputs_) {
			if (!output->no_user())
				return true;
		}

		return false;
	}

	inline size_t
	noperands() const noexcept
	{
		return operation_->narguments();
	}

	jive::input *
	add_input(const jive::base::type * type, jive::output * origin);

	jive::input *
	add_input(jive::gate * gate, jive::output * origin);

	void
	remove_input(size_t index);

	jive::output *
	add_output(const jive::base::type * type);

	inline jive::output *
	add_output(const struct jive_resource_class * rescls)
	{
		jive::output * output = add_output(jive_resource_class_get_type(rescls));
		output->required_rescls = rescls;
		return output;
	}

	jive::output *
	add_output(jive::gate * gate);

	void
	remove_output(size_t index);

	inline jive_graph *
	graph() const noexcept
	{
		return graph_;
	}

	inline jive_region *
	region() const noexcept
	{
		return region_;
	}

	inline size_t
	ninputs() const noexcept
	{
		return inputs_.size();
	}

	inline size_t
	noutputs() const noexcept
	{
		return outputs_.size();
	}

	jive_node *
	copy(jive_region * region, const std::vector<jive::output*> & operands) const;

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

		The given substitution map is updated so that all
		outputs of the original node will be substituted by
		corresponding outputs of the newly created node in
		subsequent \ref copy operations.
	*/
	jive_node *
	copy(jive_region * region, jive::substitution_map & smap) const;

	inline jive::input *
	input(size_t index) const noexcept
	{
		JIVE_DEBUG_ASSERT(index < inputs_.size());
		return inputs_[index];
	}

	inline jive::output *
	output(size_t index) const noexcept
	{
		JIVE_DEBUG_ASSERT(index < outputs_.size());
		return outputs_[index];
	}

	inline size_t
	depth() const noexcept
	{
		return depth_;
	}

	/*
		FIXME: privatize or completely remove it again
	*/
	void
	recompute_depth();

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
	jive::detail::intrusive_list_anchor<jive_node> region_node_list_anchor_;

public:
	typedef jive::detail::intrusive_list_accessor<
		jive_node,
		&jive_node::region_node_list_anchor_
	> region_node_list_accessor;

private:
	size_t depth_;
	jive_graph * graph_;
	jive_region * region_;
	std::vector<jive::input*> inputs_;
	std::vector<jive::output*> outputs_;
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

/**
	\brief Check if an edge may be added to the node
	\param self Target node
	\param origin Origin of edge
	
	Check whether an edge to @c self as target node may be added,
	originating at port @c origin.
*/
bool
jive_node_valid_edge(const jive_node * self, const jive::output * origin);

JIVE_EXPORTED_INLINE jive::input *
jive_node_get_gate_input(const jive_node * self, const jive::gate * gate)
{
	for (size_t n = 0; n < self->ninputs(); n++) {
		if (self->input(n)->gate() == gate) {
			return self->input(n);
		}
	}
	return nullptr;
}

JIVE_EXPORTED_INLINE jive::input *
jive_node_get_gate_input(const jive_node * self, const char * name)
{
	for (size_t n = 0; n < self->ninputs(); n++) {
		jive::input * i = self->input(n);
		if (i->gate() && i->gate()->name() == name)
			return i;
	}
	return nullptr;
}

JIVE_EXPORTED_INLINE jive::output *
jive_node_get_gate_output(const jive_node * self, const jive::gate * gate)
{
	for (size_t n = 0; n < self->noutputs(); n++) {
		if (self->output(n)->gate == gate) {
			return self->output(n);
		}
	}
	return nullptr;
}

JIVE_EXPORTED_INLINE jive::output *
jive_node_get_gate_output(const jive_node * self, const char * name)
{
	for (size_t n = 0; n < self->noutputs(); n++) {
		jive::output * o = self->output(n);
		if (o->gate && o->gate->name() == name)
			return o;
	}
	return nullptr;
}

JIVE_EXPORTED_INLINE jive_region *
jive_node_anchored_region(const jive_node * self, size_t index)
{
	jive_region * region = self->input(index)->origin()->node()->region();
	/* the given region can only be different if the identified input
	 * is of "anchor" type, so this implicitly checks the type */
	JIVE_DEBUG_ASSERT(self->region() != region);
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

JIVE_EXPORTED_INLINE std::vector<jive::output *>
jive_node_arguments(jive_node * self)
{
	std::vector<jive::output *> arguments;
	for (size_t n = 0; n < self->noperands(); ++n) {
		arguments.push_back(self->input(n)->origin());
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
	jive_region * region,
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

#endif
