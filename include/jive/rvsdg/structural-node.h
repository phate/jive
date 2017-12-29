/*
 * Copyright 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_RVSDG_STRUCTURAL_NODE_H
#define JIVE_RVSDG_STRUCTURAL_NODE_H

#include <jive/rvsdg/node.h>
#include <jive/rvsdg/region.h>

namespace jive {

/* structural node */

class structural_input;
class structural_op;
class structural_output;

class structural_node : public node {
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

typedef jive::detail::intrusive_list<
	jive::argument,
	jive::argument::structural_input_accessor
> argument_list;

class structural_input : public input {
	friend structural_node;
public:
	virtual
	~structural_input() noexcept;

protected:
	structural_input(
		jive::structural_node * node,
		size_t index,
		jive::output * origin,
		const jive::port & port);

public:
	virtual jive::structural_node *
	node() const noexcept override;

	argument_list arguments;

private:
	jive::structural_node * node_;
};

/* structural output class */

typedef jive::detail::intrusive_list<
	jive::result,
	jive::result::structural_output_accessor
> result_list;

class structural_output : public output {
	friend structural_node;

public:
	virtual
	~structural_output() noexcept;

protected:
	structural_output(
		jive::structural_node * node,
		size_t index,
		const jive::port & port);

public:
	virtual jive::structural_node *
	node() const noexcept override;

	result_list results;

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

/* other functions */

template <class T> static inline bool
contains(const jive::region * region, bool recursive)
{
	for (const auto & node : region->nodes) {
		if (is_opnode<T>(node))
			return true;


		auto structnode = dynamic_cast<const jive::structural_node*>(node);
		if (recursive && structnode) {
			for (size_t n = 0; n < structnode->nsubregions(); n++)
				return contains<T>(structnode->subregion(n), recursive);
		}
	}

	return false;
}

}

#endif
