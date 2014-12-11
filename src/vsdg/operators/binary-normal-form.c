/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/operators/binary-normal-form.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/operators/binary.h>
#include <jive/vsdg/operators/reduction-helpers.h>
#include <jive/vsdg/region.h>

namespace jive {

namespace {

bool
test_reduce_operands(
	const jive::base::binary_op & op,
	const std::vector<jive::output *> & args)
{
	/* pair-wise reduce */
	if (op.is_commutative()) {
		return base::detail::commutative_pairwise_test_reduce(
			args,
			[&op](jive::output * arg1, jive::output * arg2)
			{
				return op.can_reduce_operand_pair(arg1, arg2) != jive_binop_reduction_none;
			});
	} else {
		return base::detail::pairwise_test_reduce(
			args,
			[&op](jive::output * arg1, jive::output * arg2)
			{
				return op.can_reduce_operand_pair(arg1, arg2) != jive_binop_reduction_none;
			});
	}
}

std::vector<jive::output *>
reduce_operands(
	const jive::base::binary_op & op,
	std::vector<jive::output *> args)
{
	/* pair-wise reduce */
	if (op.is_commutative()) {
		return base::detail::commutative_pairwise_reduce(
			std::move(args),
			[&op](jive::output * arg1, jive::output * arg2)
			{
				jive_binop_reduction_path_t reduction =
					op.can_reduce_operand_pair(arg1, arg2);
				return reduction != jive_binop_reduction_none
					? op.reduce_operand_pair(reduction, arg1, arg2)
					: nullptr;
			});
	} else {
		return base::detail::pairwise_reduce(
			std::move(args),
			[&op](jive::output * arg1, jive::output * arg2)
			{
				jive_binop_reduction_path_t reduction =
					op.can_reduce_operand_pair(arg1, arg2);
				return reduction != jive_binop_reduction_none
					? op.reduce_operand_pair(reduction, arg1, arg2)
					: nullptr;
			});
	}
}

}

binary_normal_form::~binary_normal_form() noexcept
{
}

binary_normal_form::binary_normal_form(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive_graph * graph)
	: node_normal_form(operator_class, parent, graph)
	, enable_reducible_(true)
	, enable_reorder_(true)
	, enable_flatten_(true)
	, enable_distribute_(true)
	, enable_factorize_(true)
{
	if (auto p = dynamic_cast<binary_normal_form *>(parent)) {
		enable_reducible_ = p->enable_reducible_;
		enable_reorder_ = p->enable_reorder_;
		enable_flatten_ = p->enable_flatten_;
		enable_distribute_ = p->enable_distribute_;
		enable_factorize_ = p->enable_factorize_;
	}
}

bool
binary_normal_form::normalize_node(jive_node * node) const
{
	const jive::operation & base_op = node->operation();
	const jive::base::binary_op & op = *static_cast<const jive::base::binary_op *>(&base_op);
	
	return normalize_node(node, op);
}

bool
binary_normal_form::normalize_node(jive_node * node, const base::binary_op & op) const
{
	if (!get_mutable()) {
		return true;
	}

	std::vector<jive::output *> args = jive_node_arguments(node);
	std::vector<jive::output *> new_args;

	/* possibly expand associative */
	if (get_flatten() && op.is_associative()) {
		new_args = base::detail::associative_flatten(
			args,
			[&op](jive::output * arg) {
				const base::flattened_binary_op * fb_op = dynamic_cast<const base::flattened_binary_op *>(&arg->node()->operation());
				return arg->node()->operation() == op ||
					(fb_op && fb_op->bin_operation() == op);
			});
	} else {
		new_args = args;
	}

	if (get_reducible()) {
		new_args = reduce_operands(op, std::move(new_args));

		if (new_args.size() == 1) {
			jive_output_replace(node->outputs[0], new_args[0]);
			/* FIXME: not sure whether "destroy" is really appropriate? */
			jive_node_destroy(node);
			return false;
		}
	}
	
	/* FIXME: reorder for commutative operation */
	
	/* FIXME: attempt distributive transform */
	
	bool changes = (args != new_args);

	if (changes) {
		jive_node * new_node = nullptr;

		std::unique_ptr<operation> tmp_op;
		if (new_args.size() > 2) {
			tmp_op.reset(new base::flattened_binary_op(op, new_args.size()));
		}
		const operation & new_op =
			tmp_op ? *tmp_op : static_cast<const operation &>(op);
		if (get_cse()) {
			jive_node_cse(node->region, new_op, new_args);
		}

		JIVE_DEBUG_ASSERT(new_args.size() >= 2);
		if (!new_node) {
			new_node = new_op.create_node(node->region, new_args.size(), &new_args[0]);
		}

		if (new_node != node) {
			jive_output_replace(node->outputs[0], new_node->outputs[0]);
			jive_node_destroy(node);
			return false;
		}
	}

	return true;
}

bool
binary_normal_form::operands_are_normalized(
	const jive::operation & base_op,
	const std::vector<jive::output *> & args) const
{
	const jive::base::binary_op& op = *static_cast<const jive::base::binary_op*>(&base_op);
	if (!get_mutable()) {
		return true;
	}
	
	/* possibly expand associative */
	if (get_flatten() && op.is_associative()) {
		bool can_flatten = base::detail::associative_test_flatten(
			args,
			[&op](jive::output * arg) {
				const base::flattened_binary_op * fb_op = dynamic_cast<const base::flattened_binary_op *>(&arg->node()->operation());
				return arg->node()->operation() == op ||
					(fb_op && fb_op->bin_operation() == op);
			});
		if (can_flatten) {
			return false;
		}
	}
	
	if (get_reducible()) {
		if (test_reduce_operands(op, args)) {
			return false;
		}
	}
	
	/* FIXME: reorder for commutative operation */
	
	/* FIXME: attempt distributive transform */
	
	return node_normal_form::operands_are_normalized(base_op, args);
}

std::vector<jive::output *>
binary_normal_form::normalized_create(
	const jive::operation & base_op,
	const std::vector<jive::output *> & args) const
{
	const jive::base::binary_op& op = *static_cast<const jive::base::binary_op *>(&base_op);

	std::vector<jive::output *> new_args;

	/* possibly expand associative */
	if (get_mutable() && get_flatten() && op.is_associative()) {
		new_args = base::detail::associative_flatten(
			args,
			[&op](jive::output * arg) {
				const base::flattened_binary_op * fb_op = dynamic_cast<const base::flattened_binary_op *>(&arg->node()->operation());
				return arg->node()->operation() == op ||
					(fb_op && fb_op->bin_operation() == op);
			});
	} else {
		new_args = args;
	}

	if (get_mutable() && get_reducible()) {
		new_args = reduce_operands(op, std::move(new_args));
		if (new_args.size() == 1) {
			return std::move(new_args);
		}
	}

	/* FIXME: reorder for commutative operation */

	/* FIXME: attempt distributive transform */
	std::unique_ptr<operation> tmp_op;
	if (new_args.size() > 2) {
		tmp_op.reset(new base::flattened_binary_op(op, new_args.size()));
	}
	const operation & new_op =
		tmp_op ? *tmp_op : static_cast<const operation &>(op);

	return node_normal_form::normalized_create(new_op, new_args);
}

void
binary_normal_form::set_reducible(bool enable)
{
	if (get_reducible() == enable) {
		return;
	}

	children_set<binary_normal_form, &binary_normal_form::set_reducible>(enable);

	enable_reducible_ = enable;
	if (get_mutable() && enable) {
		jive_graph_mark_denormalized(graph());
	}
}

void
binary_normal_form::set_flatten(bool enable)
{
	if (get_flatten() == enable) {
		return;
	}

	children_set<binary_normal_form, &binary_normal_form::set_flatten>(enable);

	enable_flatten_ = enable;
	if (get_mutable() && enable) {
		jive_graph_mark_denormalized(graph());
	}
}

void
binary_normal_form::set_reorder(bool enable)
{
	if (get_reorder() == enable) {
		return;
	}

	children_set<binary_normal_form, &binary_normal_form::set_reorder>(enable);
	
	enable_reorder_ = enable;
	if (get_mutable() && enable) {
		jive_graph_mark_denormalized(graph());
	}
}

void
binary_normal_form::set_distribute(bool enable)
{
	if (get_distribute() == enable) {
		return;
	}

	children_set<binary_normal_form, &binary_normal_form::set_distribute>(enable);

	enable_distribute_ = enable;
	if (get_mutable() && enable) {
		jive_graph_mark_denormalized(graph());
	}
}

void
binary_normal_form::set_factorize(bool enable)
{
	if (get_factorize() == enable) {
		return;
	}

	children_set<binary_normal_form, &binary_normal_form::set_factorize>(enable);

	enable_factorize_ = enable;
	if (get_mutable() && enable) {
		jive_graph_mark_denormalized(graph());
	}
}


flattened_binary_normal_form::~flattened_binary_normal_form() noexcept
{
}

flattened_binary_normal_form::flattened_binary_normal_form(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive_graph * graph)
	: node_normal_form(operator_class, parent, graph)
{
}

bool
flattened_binary_normal_form::normalize_node(jive_node * node) const
{
	const base::flattened_binary_op & op =
		static_cast<const base::flattened_binary_op &>(node->operation());

	const node_normal_form * nf = jive_graph_get_nodeclass_form(graph(), typeid(op.bin_operation()));

	return static_cast<const binary_normal_form *>(nf)->normalize_node(node, op.bin_operation());
}

bool
flattened_binary_normal_form::operands_are_normalized(
	const jive::operation & base_op,
	const std::vector<jive::output *> & arguments) const
{
	const base::flattened_binary_op & op =
		static_cast<const base::flattened_binary_op &>(base_op);

	const node_normal_form * nf = jive_graph_get_nodeclass_form(graph(), typeid(op.bin_operation()));

	return nf->operands_are_normalized(op.bin_operation(), arguments);
}

std::vector<jive::output *>
flattened_binary_normal_form::normalized_create(
	const jive::operation & base_op,
	const std::vector<jive::output *> & arguments) const
{
	const base::flattened_binary_op & op =
		static_cast<const base::flattened_binary_op &>(base_op);

	const node_normal_form * nf = jive_graph_get_nodeclass_form(graph(), typeid(op.bin_operation()));

	return nf->normalized_create(op.bin_operation(), arguments);
}

}
