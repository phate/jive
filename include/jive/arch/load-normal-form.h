/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_LOAD_NORMAL_FORM_H
#define JIVE_ARCH_LOAD_NORMAL_FORM_H

#include <jive/vsdg/node-normal-form.h>

namespace jive {

class load_normal_form : public node_normal_form {
public:
	virtual
	~load_normal_form() noexcept;

	load_normal_form(
		const jive_node_class * node_class,
		jive::node_normal_form * parent,
		jive_graph * graph) noexcept;

	virtual bool
	normalize_node(jive_node * node) const override;

	virtual bool
	operands_are_normalized(
		const jive::operation & op,
		const std::vector<jive::output *> & arguments) const override;

	virtual std::vector<jive::output *>
	normalized_create(
		const jive::operation & op,
		const std::vector<jive::output *> & arguments) const override;

	virtual void
	set_reducible(bool enable);
	inline bool
	get_reducible() const noexcept { return enable_reducible_; }

private:
	bool enable_reducible_;
};

}

/* load node normal form */

extern const jive_node_normal_form_class JIVE_LOAD_NORMAL_FORM;

#endif
