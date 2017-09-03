/*
 * Copyright 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_SIMPLE_NODE_H
#define JIVE_VSDG_SIMPLE_NODE_H

#include <jive/vsdg/node.h>

namespace jive {

class simple_op;
class simple_node;

/* inputs */

class simple_input final : public input {
	friend jive::output;

public:
	virtual
	~simple_input() noexcept;

	simple_input(
		jive::simple_node * node,
		size_t index,
		jive::output * origin,
		const jive::port & port);

public:
	virtual jive::node *
	node() const noexcept override;

private:
	jive::simple_node * node_;
};

/* outputs */

class simple_output final : public output {
	friend jive::simple_input;

public:
	virtual
	~simple_output() noexcept;

	simple_output(
		jive::simple_node * node,
		size_t index,
		const jive::port & port);

public:
	virtual jive::node *
	node() const noexcept override;

private:
	jive::simple_node * node_;
};

/* simple nodes */

class simple_node final : public node {
public:
	virtual
	~simple_node();

	simple_node(
		const jive::operation & op,
		jive::region * region,
		const std::vector<jive::output*> & operands);

	inline jive::simple_input *
	input(size_t index) const noexcept
	{
		return static_cast<simple_input*>(node::input(index));
	}

	inline jive::simple_output *
	output(size_t index) const noexcept
	{
		return static_cast<simple_output*>(node::output(index));
	}

	virtual jive::node *
	copy(jive::region * region, const std::vector<jive::output*> & operands) const override;

	virtual jive::node *
	copy(jive::region * region, jive::substitution_map & smap) const override;
};

std::vector<jive::output*>
create_normalized(
	jive::region * region,
	const jive::simple_op & op,
	const std::vector<jive::output*> & arguments);

}

#endif
