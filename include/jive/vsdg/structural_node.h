/*
 * Copyright 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_STRUCTURAL_NODE_H
#define JIVE_VSDG_STRUCTURAL_NODE_H

#include <jive/vsdg/node.h>

namespace jive {

class argument;
class result;
class structural_node;

class structural_input final : public iport {
	friend structural_node;

public:
	virtual
	~structural_input() noexcept;

private:
	structural_input(
		jive::structural_node * node,
		size_t index,
		jive::oport * origin,
		const jive::base::type & type);

	structural_input(
		jive::structural_node * node,
		size_t index,
		jive::oport * origin,
		jive::gate * gate);

	structural_input(
		jive::structural_node * node,
		size_t index,
		jive::oport * origin,
		const struct jive_resource_class * rescls);

public:
	virtual jive::node *
	node() const noexcept override;

	struct {
		jive::argument * first;
		jive::argument * last;
	} arguments;

private:
	jive::structural_node * node_;
};

class structural_output final : public oport {
	friend structural_node;

public:
	virtual
	~structural_output() noexcept;

private:
	structural_output(
		jive::structural_node * node,
		size_t index,
		const jive::base::type & type);

	structural_output(
		jive::structural_node * node,
		size_t index,
		jive::gate * gate);

	structural_output(
		jive::structural_node * node,
		size_t index,
		const struct jive_resource_class * rescls);

public:
	virtual const jive::base::type &
	type() const noexcept override;

	virtual jive::region *
	region() const noexcept override;

	virtual jive::node *
	node() const noexcept override;

	struct {
		jive::result * first;
		jive::result * last;
	} results;

private:
	jive::structural_node * node_;
	std::unique_ptr<jive::base::type> type_;
};

class structural_node final : public node {
public:
	virtual
	~structural_node();

	structural_node(
		const jive::operation & op,
		jive::region * region,
		size_t nsubregions);

	virtual bool
	has_users() const noexcept override;

	virtual bool
	has_successors() const noexcept override;

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

	virtual size_t
	ninputs() const noexcept override;

	virtual jive::structural_input *
	input(size_t index) const noexcept override;

	virtual size_t
	noutputs() const noexcept override;

	virtual jive::structural_output *
	output(size_t index) const noexcept override;

	virtual jive::structural_input *
	add_input(const jive::base::type * type, jive::oport * origin) override;

	virtual jive::structural_input *
	add_input(jive::gate * gate, jive::oport * origin) override;

	virtual jive::structural_input *
	add_input(const struct jive_resource_class * rescls, jive::oport * origin) override;

	virtual void
	remove_input(size_t index) override;

	virtual jive::structural_output *
	add_output(const jive::base::type * type) override;

	virtual jive::structural_output *
	add_output(const struct jive_resource_class * rescls) override;

	virtual jive::structural_output *
	add_output(jive::gate * gate) override;

	virtual void
	remove_output(size_t index) override;

	virtual jive::structural_node *
	copy(jive::region * region, const std::vector<jive::oport*> & operands) const override;

	virtual jive::structural_node *
	copy(jive::region * region, jive::substitution_map & smap) const override;

private:
	std::vector<std::unique_ptr<jive::region>> subregions_;
	std::vector<std::unique_ptr<structural_input>> inputs_;
	std::vector<std::unique_ptr<structural_output>> outputs_;
};

}

#endif
