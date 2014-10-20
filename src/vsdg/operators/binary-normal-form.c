/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/operators/binary-normal-form.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/operators/binary.h>
#include <jive/vsdg/region.h>

namespace jive {

namespace {

std::vector<jive::output *>
reduce_operands(
	const jive::base::binary_op & op,
	std::vector<jive::output *> args)
{
	/* pair-wise reduce */
	if (op.is_commutative()) {
		size_t n = 0;
		while (n < args.size()) {
			size_t k = n + 1;
			while (k < args.size()) {
				jive::output * op1 = args[n];
				jive::output * op2 = args[k];
				jive_binop_reduction_path_t reduction = op.can_reduce_operand_pair(op1, op2);
				if (reduction != jive_binop_reduction_none) {
					args.erase(args.begin() + k);
					args[n] = op.reduce_operand_pair(reduction, op1, op2);
					--n;
					break;
				}
				++k;;
			}
			++n;
		}
	} else {
		size_t n = 0;
		while (n + 1 < args.size()) {
			jive_binop_reduction_path_t reduction =
				op.can_reduce_operand_pair(args[n], args[n + 1]);
			if (reduction != jive_binop_reduction_none) {
				args[n] = op.reduce_operand_pair(reduction, args[n], args[n + 1]);
				args.erase(args.begin() + n + 1);
				if (n > 0) {
					--n;
				}
			} else {
				++n;
			}
		}
	}
	return args;
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
	if (!get_mutable()) {
		return true;
	}

	const jive::operation & base_op = node->operation();
	const jive::base::binary_op & op = *static_cast<const jive::base::binary_op *>(&base_op);
	
	std::vector<jive::output *> args;
	for (size_t n = 0; n < node->ninputs; ++n) {
		args.push_back(node->inputs[n]->origin());
	}

	std::vector<jive::output *> new_args;

	/* possibly expand associative */
	if (get_flatten() && op.is_associative()) {
		for (jive::output * arg : args) {
			// FIXME: switch to comparing operator, not just typeid, after
			// converting "concat" to not be a binary operator anymore
			if (typeid(arg->node()->operation()) == typeid(op)) {
				for (size_t k = 0; k < arg->node()->ninputs; ++k) {
					new_args.push_back(arg->node()->inputs[k]->origin());
				}
			} else {
				new_args.push_back(arg);
			}
		}
	} else {
		new_args = args;
	}

	if (get_reducible()) {
		new_args = reduce_operands(op, new_args);

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

		if (get_cse()) {
			jive_node_cse(node->region, op, new_args);
		}

		JIVE_DEBUG_ASSERT(new_args.size() >= 2);
		if (!new_node) {
			new_node = op.create_node(node->region, new_args.size(), &new_args[0]);
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
		size_t n;
		for (jive::output * arg : args) {
			// FIXME: switch to comparing operator, not just typeid, after
			// converting "concat" to not be a binary operator anymore
			if (typeid(arg->node()->operation()) == typeid(op)) {
				return false;
			}
		}
	}
	
	if (get_reducible()) {
		std::vector<jive::output *> new_args =
			reduce_operands(op, args);
		
		if (new_args != args) {
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
		for (jive::output * arg : args) {
			// FIXME: switch to comparing operator, not just typeid, after
			// converting "concat" to not be a binary operator anymore
			if (typeid(arg->node()->operation()) == typeid(op)) {
				for(size_t k = 0; k < arg->node()->noperands; k++)
					new_args.push_back(arg->node()->inputs[k]->origin());
			} else {
				new_args.push_back(arg);
			}
		}
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

	return node_normal_form::normalized_create(op, new_args);
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

}
