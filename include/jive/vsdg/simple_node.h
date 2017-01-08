/*
 * Copyright 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_SIMPLE_NODE_H
#define JIVE_VSDG_SIMPLE_NODE_H

#include <jive/vsdg/node.h>

namespace jive {

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
	has_successors() const noexcept override;

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
