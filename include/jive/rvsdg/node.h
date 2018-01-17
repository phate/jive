/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_RVSDG_NODE_H
#define JIVE_RVSDG_NODE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <unordered_set>
#include <utility>

#include <jive/rvsdg/operation.h>
#include <jive/rvsdg/resource.h>
#include <jive/util/intrusive-list.h>
#include <jive/util/strfmt.h>

namespace jive {
namespace base {
	class type;
}

class gate;
class graph;
class node_normal_form;
class output;
class substitution_map;

/* inputs */

class input {
	friend jive::node;
	friend jive::region;

	jive::detail::intrusive_list_anchor<
		jive::input
	> gate_input_anchor_;

public:
	typedef jive::detail::intrusive_list_accessor<
		jive::input,
		&jive::input::gate_input_anchor_
	> gate_input_accessor;

	virtual
	~input() noexcept;

	input(
		size_t index,
		jive::output * origin,
		jive::region * region,
		const jive::port & port);

	input(const input &) = delete;

	input(input &&) = delete;

	input &
	operator=(const input &) = delete;

	input &
	operator=(input &&) = delete;

	inline size_t
	index() const noexcept
	{
		return index_;
	}

	jive::output *
	origin() const noexcept
	{
		return origin_;
	}

	void
	divert_to(jive::output * new_origin);

	inline const jive::type &
	type() const noexcept
	{
		return port_.type();
	}

	inline jive::region *
	region() const noexcept
	{
		return region_;
	}

	inline const jive::port &
	port() const noexcept
	{
		return port_;
	}

	virtual jive::node *
	node() const noexcept = 0;

	virtual std::string
	debug_string() const;

private:
	size_t index_;
	jive::port port_;
	jive::output * origin_;
	jive::region * region_;
};

/* outputs */

class output {
	friend input;
	friend jive::node;
	friend jive::region;

	typedef std::unordered_set<jive::input*>::const_iterator user_iterator;

	jive::detail::intrusive_list_anchor<
		jive::output
	> gate_output_anchor_;

public:
	typedef jive::detail::intrusive_list_accessor<
		jive::output,
		&jive::output::gate_output_anchor_
	> gate_output_accessor;

	virtual
	~output() noexcept;

	output(
		size_t index,
		jive::region * region,
		const jive::port & port);

	output(const output &) = delete;

	output(output &&) = delete;

	output &
	operator=(const output &) = delete;

	output &
	operator=(output &&) = delete;

	inline size_t
	index() const noexcept
	{
		return index_;
	}

	inline size_t
	nusers() const noexcept
	{
		return users_.size();
	}

	inline void
	replace(jive::output * new_origin)
	{
		if (this == new_origin)
			return;

		while (users_.size())
			(*users_.begin())->divert_to(new_origin);
	}

	inline user_iterator
	begin() const noexcept
	{
		return users_.begin();
	}

	inline user_iterator
	end() const noexcept
	{
		return users_.end();
	}

	inline const jive::type &
	type() const noexcept
	{
		return port_.type();
	}

	inline jive::region *
	region() const noexcept
	{
		return region_;
	}

	inline const jive::port &
	port() const noexcept
	{
		return port_;
	}

	virtual jive::node *
	node() const noexcept = 0;

	virtual std::string
	debug_string() const;

private:
	void
	remove_user(jive::input * user);

	void
	add_user(jive::input * user);

	size_t index_;
	jive::port port_;
	jive::region * region_;
	std::unordered_set<jive::input*> users_;
};

class node {
public:
	virtual
	~node();

	node(std::unique_ptr<jive::operation> op, jive::region * region);

	inline const jive::operation &
	operation() const noexcept
	{
		return *operation_;
	}

	inline bool
	has_users() const noexcept
	{
		for (const auto & output : outputs_) {
			if (output->nusers() != 0)
				return true;
		}

		return false;
	}

	inline bool
	has_predecessors() const noexcept
	{
		for (const auto & input : inputs_) {
			if (input->origin()->node())
				return true;
		}

		return false;
	}

	inline bool
	has_successors() const noexcept
	{
		for (const auto & output : outputs_) {
			for (const auto & user : *output) {
				if (user->node())
					return true;
			}
		}

		return false;
	}

	inline size_t
	ninputs() const noexcept
	{
		return inputs_.size();
	}

	inline jive::input *
	input(size_t index) const noexcept
	{
		JIVE_DEBUG_ASSERT(index < ninputs());
		return inputs_[index].get();
	}

	inline size_t
	noutputs() const noexcept
	{
		return outputs_.size();
	}

	inline jive::output *
	output(size_t index) const noexcept
	{
		JIVE_DEBUG_ASSERT(index < noutputs());
		return outputs_[index].get();
	}

protected:
	void
	add_input(std::unique_ptr<jive::input> input);

	void
	remove_input(size_t index);

	inline void
	add_output(std::unique_ptr<jive::output> output)
	{
		outputs_.push_back(std::move(output));
	}

	void
	remove_output(size_t index);

public:
	inline jive::graph *
	graph() const noexcept
	{
		return graph_;
	}

	inline jive::region *
	region() const noexcept
	{
		return region_;
	}

	virtual jive::node *
	copy(jive::region * region, const std::vector<jive::output*> & operands) const = 0;

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
	virtual jive::node *
	copy(jive::region * region, jive::substitution_map & smap) const = 0;

	inline size_t
	depth() const noexcept
	{
		return depth_;
	}

	void
	recompute_depth(jive::input * input);

private:
	jive::detail::intrusive_list_anchor<
		jive::node
	> region_node_list_anchor_;

	jive::detail::intrusive_list_anchor<
		jive::node
	> region_top_node_list_anchor_;

	jive::detail::intrusive_list_anchor<
		jive::node
	> region_bottom_node_list_anchor_;

public:
	typedef jive::detail::intrusive_list_accessor<
		jive::node,
		&jive::node::region_node_list_anchor_
	> region_node_list_accessor;

	typedef jive::detail::intrusive_list_accessor<
		jive::node,
		&jive::node::region_top_node_list_anchor_
	> region_top_node_list_accessor;

	typedef jive::detail::intrusive_list_accessor<
		jive::node,
		&jive::node::region_bottom_node_list_anchor_
	> region_bottom_node_list_accessor;

private:
	size_t depth_;
	jive::graph * graph_;
	jive::region * region_;
	std::unique_ptr<jive::operation> operation_;
	std::vector<std::unique_ptr<jive::input>> inputs_;
	std::vector<std::unique_ptr<jive::output>> outputs_;
};

static inline std::vector<jive::output*>
operands(const jive::node * node)
{
	std::vector<jive::output*> operands;
	for (size_t n = 0; n < node->ninputs(); n++)
		operands.push_back(node->input(n)->origin());
	return operands;
}

static inline std::vector<jive::output*>
outputs(const jive::node * node)
{
	std::vector<jive::output*> outputs;
	for (size_t n = 0; n < node->noutputs(); n++)
		outputs.push_back(node->output(n));
	return outputs;
}

static inline void
replace(
	jive::node * node,
	const std::vector<jive::output*> & outputs)
{
	JIVE_DEBUG_ASSERT(node->noutputs() == outputs.size());

	for (size_t n = 0; n < outputs.size(); n++)
		node->output(n)->replace(outputs[n]);
}

template <class T> static inline bool
is_opnode(const jive::node * node) noexcept
{
	static_assert(std::is_base_of<jive::operation, T>::value,
		"Template parameter T must be derived from jive::operation.");

	if (!node)
		return false;

	return dynamic_cast<const T*>(&node->operation()) != nullptr;
}

jive::node *
producer(const jive::output * output) noexcept;

bool
normalize(jive::node * node);

}

static inline jive::output *
jive_node_get_gate_output(const jive::node * self, const jive::gate * gate)
{
	for (size_t n = 0; n < self->noutputs(); n++) {
		if (self->output(n)->port().gate() == gate) {
			return self->output(n);
		}
	}
	return nullptr;
}

#endif
