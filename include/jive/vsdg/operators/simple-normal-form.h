/*
 * Copyright 2017 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_OPERATORS_SIMPLE_NORMAL_FORM_H
#define JIVE_VSDG_OPERATORS_SIMPLE_NORMAL_FORM_H

#include <jive/vsdg/node-normal-form.h>

namespace jive {

class simple_normal_form : public node_normal_form {
public:
	virtual
	~simple_normal_form() noexcept;

	simple_normal_form(
		const std::type_info & operator_class,
		jive::node_normal_form * parent,
		jive::graph * graph) noexcept;

	virtual bool
	normalize_node(jive::node * node) const;

	virtual bool
	operands_are_normalized(
		const jive::operation & op,
		const std::vector<jive::oport*> & arguments) const;

	virtual std::vector<jive::oport*>
	normalized_create(
		jive::region * region,
		const jive::operation & op,
		const std::vector<jive::oport*> & arguments) const;

	virtual void
	set_cse(bool enable);

	inline bool
	get_cse() const noexcept
	{
		return enable_cse_;
	}

private:
	bool enable_cse_;
};

}

#endif
