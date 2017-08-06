/*
 * Copyright 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_SIMPLE_NODE_H
#define JIVE_VSDG_SIMPLE_NODE_H

#include <jive/vsdg/node.h>

namespace jive {

/* inputs */

class simple_input final : public iport {
	friend jive::output;

public:
	virtual
	~simple_input() noexcept;

	simple_input(
		jive::node * node,
		size_t index,
		jive::oport * origin,
		const jive::base::type & type);

	simple_input(
		jive::node * node,
		size_t index,
		jive::oport * origin,
		jive::gate * gate);

	simple_input(
		jive::node * node,
		size_t index,
		jive::oport * origin,
		const struct jive_resource_class * rescls);

public:
	virtual jive::node *
	node() const noexcept override;

private:
	jive::node * node_;
};

/* outputs */

class simple_output final : public oport {
	friend jive::simple_input;

public:
	virtual
	~simple_output() noexcept;

	simple_output(jive::node * node, size_t index, const jive::base::type & type);

	simple_output(jive::node * node, size_t index, jive::gate * gate);

	simple_output(
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

	virtual bool
	has_users() const noexcept override;

	virtual bool
	has_successors() const noexcept override;

	virtual size_t
	ninputs() const noexcept override;

	virtual jive::simple_input *
	input(size_t index) const noexcept override;

	virtual size_t
	noutputs() const noexcept override;

	virtual jive::simple_output *
	output(size_t index) const noexcept override;

	virtual jive::simple_input *
	add_input(const jive::base::type * type, jive::oport * origin) override;

	virtual jive::simple_input *
	add_input(jive::gate * gate, jive::oport * origin) override;

	virtual jive::simple_input *
	add_input(const struct jive_resource_class * rescls, jive::oport * origin) override;

	virtual void
	remove_input(size_t index) override;

	virtual jive::simple_output *
	add_output(const jive::base::type * type) override;

	virtual jive::simple_output *
	add_output(const struct jive_resource_class * rescls) override;

	virtual jive::simple_output *
	add_output(jive::gate * gate) override;

	virtual void
	remove_output(size_t index) override;

	virtual jive::node *
	copy(jive::region * region, const std::vector<jive::oport*> & operands) const override;

	virtual jive::node *
	copy(jive::region * region, jive::substitution_map & smap) const override;

private:
	std::vector<std::unique_ptr<jive::simple_input>> inputs_;
	std::vector<std::unique_ptr<jive::simple_output>> outputs_;
};

std::vector<jive::oport*>
create_normalized(
	jive::region * region,
	const jive::operation & op,
	const std::vector<jive::oport*> & arguments);

}

#endif
