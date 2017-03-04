/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_OPERATORS_BINARY_NORMAL_FORM_H
#define JIVE_VSDG_OPERATORS_BINARY_NORMAL_FORM_H

#include <jive/common.h>
#include <jive/vsdg/operators/simple-normal-form.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators/operation.h>
#include <jive/vsdg/operators/binary.h>

namespace jive {

class binary_normal_form final : public simple_normal_form {
public:
	virtual
	~binary_normal_form() noexcept;

	binary_normal_form(
		const std::type_info & operator_class,
		jive::node_normal_form * parent,
		jive::graph * graph);

	virtual bool
	normalize_node(jive::node * node) const override;

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

	virtual void
	set_flatten(bool enable);
	inline bool
	get_flatten() const noexcept { return enable_flatten_; }

	virtual void
	set_reorder(bool enable);
	inline bool
	get_reorder() const noexcept { return enable_reorder_; }

	virtual void
	set_distribute(bool enable);
	inline bool
	get_distribute() const noexcept { return enable_distribute_; }

	virtual void
	set_factorize(bool enable);
	inline bool
	get_factorize() const noexcept { return enable_factorize_; }

private:
	bool
	normalize_node(jive::node * node, const base::binary_op & op) const;

	bool enable_reducible_;
	bool enable_reorder_;
	bool enable_flatten_;
	bool enable_distribute_;
	bool enable_factorize_;

	friend class flattened_binary_normal_form;
};

class flattened_binary_normal_form final : public node_normal_form {
public:
	virtual
	~flattened_binary_normal_form() noexcept;

	flattened_binary_normal_form(
		const std::type_info & operator_class,
		jive::node_normal_form * parent,
		jive::graph * graph);

	virtual bool
	normalize_node(jive::node * node) const override;

	virtual bool
	operands_are_normalized(
		const jive::operation & op,
		const std::vector<jive::oport*> & arguments) const override;

	virtual std::vector<jive::oport*>
	normalized_create(
		jive::region * region,
		const jive::operation & op,
		const std::vector<jive::oport*> & arguments) const override;
};

}

#endif
