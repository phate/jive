/*
 * Copyright 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_SIMPLE_NODE_H
#define JIVE_VSDG_SIMPLE_NODE_H

#include <jive/vsdg/node.h>

namespace jive {

/* inputs */

class input final : public iport {
	friend jive::output;

public:
	virtual
	~input() noexcept;

	input(
		jive::node * node,
		size_t index,
		jive::oport * origin,
		const jive::base::type & type);

	input(
		jive::node * node,
		size_t index,
		jive::oport * origin,
		jive::gate * gate);

	input(
		jive::node * node,
		size_t index,
		jive::oport * origin,
		const jive::base::type & type,
		const struct jive_resource_class * rescls);

public:
	virtual const jive::base::type &
	type() const noexcept override;

	virtual jive::region *
	region() const noexcept override;

	virtual jive::node *
	node() const noexcept override;

	virtual void
	divert_origin(jive::oport * new_origin) override;

private:
	jive::node * node_;

	/*
		FIXME: This attribute is necessary as long as the number of inputs do not coincide with the
		number given by the operation. Once this is fixed, the attribute can be removed and the type
		can be taken from the operation.
	*/
	std::unique_ptr<jive::base::type> type_;
};

/* outputs */

class output final : public oport {
	friend jive::input;

public:
	virtual
	~output() noexcept;

	output(jive::node * node, size_t index, const jive::base::type & type);

	output(jive::node * node, size_t index, jive::gate * gate);

	output(
		jive::node * node,
		size_t index,
		const jive::base::type & type,
		const struct jive_resource_class * rescls);

public:
	virtual const jive::base::type &
	type() const noexcept override;

	virtual jive::region *
	region() const noexcept override;

	virtual jive::node *
	node() const noexcept override;

private:
	jive::node * node_;

	/*
		FIXME: This attribute is necessary as long as the number of inputs do not coincide with the
		number given by the operation. Once this is fixed, the attribute can be removed and the type
		can be taken from the operation.
	*/
	std::unique_ptr<jive::base::type> type_;
};

/* simple nodes */

class simple_node final : public node {
public:
	virtual
	~simple_node();

	simple_node(
		const jive::operation & op,
		jive::region * region,
		const std::vector<jive::oport*> & operands);

	virtual size_t
	depth() const noexcept override;

	virtual void
	recompute_depth() override;

	virtual bool
	has_users() const noexcept override;

	virtual size_t
	ninputs() const noexcept override;

	virtual jive::input *
	input(size_t index) const noexcept override;

	virtual size_t
	noutputs() const noexcept override;

	virtual jive::output *
	output(size_t index) const noexcept override;

	virtual jive::input *
	add_input(const jive::base::type * type, jive::oport * origin) override;

	virtual jive::input *
	add_input(jive::gate * gate, jive::oport * origin) override;

	virtual jive::input *
	add_input(const struct jive_resource_class * rescls, jive::oport * origin) override;

	virtual void
	remove_input(size_t index) override;

	virtual jive::output *
	add_output(const jive::base::type * type) override;

	virtual jive::output *
	add_output(const struct jive_resource_class * rescls) override;

	virtual jive::output *
	add_output(jive::gate * gate) override;

	virtual void
	remove_output(size_t index) override;

	virtual jive::node *
	copy(jive::region * region, const std::vector<jive::oport*> & operands) const override;

	virtual jive::node *
	copy(jive::region * region, jive::substitution_map & smap) const override;

private:
	size_t depth_;
	std::vector<std::unique_ptr<jive::input>> inputs_;
	std::vector<std::unique_ptr<jive::output>> outputs_;
};

}

#endif
