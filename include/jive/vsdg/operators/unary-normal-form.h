/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_OPERATORS_UNARY_NORMAL_FORM_H
#define JIVE_VSDG_OPERATORS_UNARY_NORMAL_FORM_H

#include <jive/common.h>
#include <jive/vsdg/node-normal-form.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators/base.h>

namespace jive {

class unary_normal_form final : public node_normal_form {
public:
	virtual
	~unary_normal_form() noexcept;

	unary_normal_form(
		const std::type_info & operator_class,
		jive::node_normal_form * parent,
		jive_graph * graph);

	virtual bool
	normalize_node(jive_node * node) const override;

	virtual bool
	operands_are_normalized(
		const jive::operation & op,
		const std::vector<jive::oport*> & arguments) const override;

	virtual std::vector<jive::oport*>
	normalized_create(
		jive::region * region,
		const jive::operation & op,
		const std::vector<jive::oport*> & arguments) const override;

	virtual void
	set_reducible(bool enable);
	inline bool
	get_reducible() const noexcept { return enable_reducible_; }

private:
	bool enable_reducible_;
};

}

#endif
