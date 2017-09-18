/*
 * Copyright 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_STRUCTURAL_NODE_H
#define JIVE_VSDG_STRUCTURAL_NODE_H

#include <jive/vsdg/node.h>

namespace jive {

/* structural node */

class argument;
class result;
class structural_input;
class structural_op;
class structural_output;

class structural_node final : public node {
public:
	virtual
	~structural_node();

	structural_node(
		const jive::structural_op & op,
		jive::region * region,
		size_t nsubregions);

	inline size_t
	nsubregions() const noexcept
	{
		return subregions_.size();
	}

	inline jive::region *
	subregion(size_t index) const noexcept
	{
		JIVE_DEBUG_ASSERT(index < nsubregions());
		return subregions_[index].get();
	}

	inline jive::structural_input *
	input(size_t index) const noexcept;

	inline jive::structural_output *
	output(size_t index) const noexcept;

	jive::structural_input *
	add_input(const jive::port & port, jive::output * origin);

	jive::structural_output *
	add_output(const jive::port & port);

	inline void
	remove_input(size_t index)
	{
		node::remove_input(index);
	}

	inline void
	remove_output(size_t index)
	{
		node::remove_output(index);
	}

	virtual jive::structural_node *
	copy(jive::region * region, const std::vector<jive::output*> & operands) const override;

	virtual jive::structural_node *
	copy(jive::region * region, jive::substitution_map & smap) const override;

private:
	std::vector<std::unique_ptr<jive::region>> subregions_;
};

/* structural input class */

class structural_input final : public input {
	friend structural_node;

public:
	virtual
	~structural_input() noexcept;

private:
	structural_input(
		jive::structural_node * node,
		size_t index,
		jive::output * origin,
		const jive::port & port);

public:
	virtual jive::structural_node *
	node() const noexcept override;

	struct {
		jive::argument * first;
		jive::argument * last;
	} arguments;

private:
	jive::structural_node * node_;
};

/* structural output class */

class structural_output final : public output {
	friend structural_node;

public:
	virtual
	~structural_output() noexcept;

private:
	structural_output(
		jive::structural_node * node,
		size_t index,
		const jive::port & port);

public:
	virtual jive::structural_node *
	node() const noexcept override;

	struct {
		jive::result * first;
		jive::result * last;
	} results;

private:
	jive::structural_node * node_;
};

/* structural node method definitions */

inline jive::structural_input *
structural_node::input(size_t index) const noexcept
{
	return static_cast<structural_input*>(node::input(index));
}

inline jive::structural_output *
structural_node::output(size_t index) const noexcept
{
	return static_cast<structural_output*>(node::output(index));
}

}

#endif
