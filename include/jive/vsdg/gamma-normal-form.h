/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_GAMMA_NORMAL_FORM_H
#define JIVE_VSDG_GAMMA_NORMAL_FORM_H

#include <jive/vsdg/anchor-normal-form.h>

/* normal form */

namespace jive {

class gamma_normal_form final : public anchor_normal_form {
public:
	virtual
	~gamma_normal_form() noexcept;

	gamma_normal_form(
		const std::type_info & operator_class,
		jive::node_normal_form * parent,
		jive_graph * graph) noexcept;

	virtual bool
	normalize_node(jive_node * node) const override;

	virtual bool
	operands_are_normalized(
		const jive::operation & op,
		const std::vector<jive::oport*> & arguments) const override;

	virtual std::vector<jive::output *>
	normalized_create(
		jive_region * region,
		const jive::operation & op,
		const std::vector<jive::oport*> & arguments) const override;

	virtual void
	set_reducible(bool enable) override;

	virtual void
	set_predicate_reduction(bool enable);
	inline bool
	get_predicate_reduction() const noexcept { return enable_predicate_reduction_; }

	virtual void
	set_invariant_reduction(bool enable);
	inline bool
	get_invariant_reduction() const noexcept { return enable_invariant_reduction_; }

private:
	bool enable_predicate_reduction_;
	bool enable_invariant_reduction_;
};

}

#endif
