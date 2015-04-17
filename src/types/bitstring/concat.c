/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/concat.h>

#include <string.h>

#include <jive/common.h>

#include <jive/types/bitstring/constant.h>
#include <jive/types/bitstring/slice.h>
#include <jive/types/bitstring/type.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/operators.h>
#include <jive/vsdg/operators/reduction-helpers.h>
#include <jive/vsdg/region.h>

jive::output *
jive_bitconcat(size_t narguments, jive::output * const * arguments)
{
	std::vector<jive::bits::type> types;
	for (size_t n = 0; n < narguments; ++n) {
		types.push_back(dynamic_cast<const jive::bits::type &>(arguments[n]->type()));
	}

	jive_graph * graph = arguments[0]->node()->graph;

	jive::bits::concat_op op(std::move(types));
	return jive_node_create_normalized(
		graph, op, std::vector<jive::output *>(arguments, arguments + narguments))[0];
}

namespace jive {
namespace bits {

namespace {

bool
concat_test_reduce_arg_pair(const jive::output * arg1, const jive::output * arg2)
{
	const constant_op * arg1_constant = dynamic_cast<const constant_op *>(
		&arg1->node()->operation());
	const constant_op * arg2_constant = dynamic_cast<const constant_op *>(
		&arg2->node()->operation());
	if (arg1_constant && arg2_constant) {
		return true;
	}

	const slice_op * arg1_slice = dynamic_cast<const slice_op *>(
		&arg1->node()->operation());
	const slice_op * arg2_slice = dynamic_cast<const slice_op *>(
		&arg2->node()->operation());
	if (arg1_slice && arg2_slice && arg1_slice->high() == arg2_slice->low() &&
		arg1->node()->inputs[0]->origin() == arg2->node()->inputs[0]->origin()) {
		return true;
	}

	return false;
}

jive::output *
concat_reduce_arg_pair(jive::output * arg1, jive::output * arg2)
{
	const constant_op * arg1_constant = dynamic_cast<const constant_op *>(
		&arg1->node()->operation());
	const constant_op * arg2_constant = dynamic_cast<const constant_op *>(
		&arg2->node()->operation());
	if (arg1_constant && arg2_constant) {
		size_t nbits = arg1_constant->value().nbits() + arg2_constant->value().nbits();
		char bits[nbits];
		memcpy(bits, &arg1_constant->value()[0], arg1_constant->value().nbits());
		memcpy(
			bits + arg1_constant->value().nbits(),
			&arg2_constant->value()[0],
			arg2_constant->value().nbits());

		return jive_bitconstant(arg1->node()->graph, nbits, bits);
	}

	const slice_op * arg1_slice = dynamic_cast<const slice_op *>(
		&arg1->node()->operation());
	const slice_op * arg2_slice = dynamic_cast<const slice_op *>(
		&arg2->node()->operation());
	if (arg1_slice && arg2_slice && arg1_slice->high() == arg2_slice->low() &&
		arg1->node()->inputs[0]->origin() == arg2->node()->inputs[0]->origin()) {
		jive::output * origin1 = arg1->node()->inputs[0]->origin();
		/* FIXME: support sign bit */
		return jive_bitslice(origin1, arg1_slice->low(), arg2_slice->high());
	}

	return nullptr;
}

std::vector<jive::bits::type>
types_from_arguments(const std::vector<jive::output *> & args)
{
	std::vector<type> types;
	for (const jive::output * arg : args) {
		types.push_back(static_cast<const type &>(arg->type()));
	}
	return types;
}

}

class concat_normal_form final : public node_normal_form {
public:
	virtual
	~concat_normal_form() noexcept;

	concat_normal_form(
		jive::node_normal_form * parent,
		jive_graph * graph)
		: node_normal_form(typeid(concat_op), parent, graph)
		, enable_reducible_(true)
		, enable_flatten_(true)
	{
	}

	virtual bool
	normalize_node(jive_node * node) const override
	{
		if (!get_mutable()) {
			return true;
		}

		std::vector<jive::output *> args = jive_node_arguments(node);
		std::vector<jive::output *> new_args;

		/* possibly expand associative */
		if (get_flatten()) {
			new_args = base::detail::associative_flatten(
				args,
				[](jive::output * arg) {
					// FIXME: switch to comparing operator, not just typeid, after
					// converting "concat" to not be a binary operator anymore
					return typeid(arg->node()->operation()) == typeid(concat_op);
				});
		} else {
			new_args = args;
		}

		if (get_reducible()) {
			new_args = base::detail::pairwise_reduce(
				std::move(new_args),
				concat_reduce_arg_pair);

			if (new_args.size() == 1) {
				node->outputs[0]->replace(new_args[0]);
				/* FIXME: not sure whether "destroy" is really appropriate? */
				jive_node_destroy(node);
				return false;
			}
		}
		
		bool changes = (args != new_args);

		if (changes) {
			jive_node * new_node = nullptr;
			concat_op op(types_from_arguments(new_args));

			if (get_cse()) {
				jive_node_cse(node->region, op, new_args);
			}

			JIVE_DEBUG_ASSERT(new_args.size() >= 2);
			if (!new_node) {
				new_node = op.create_node(node->region, new_args.size(), &new_args[0]);
			}

			if (new_node != node) {
				node->outputs[0]->replace(new_node->outputs[0]);
				jive_node_destroy(node);
				return false;
			}
		}

		return true;
	}

	virtual bool
	operands_are_normalized(
		const jive::operation & op,
		const std::vector<jive::output *> & arguments) const override
	{
		if (!get_mutable()) {
			return true;
		}
		
		/* possibly expand associative */
		if (get_flatten()) {
			bool can_flatten = base::detail::associative_test_flatten(
				arguments,
				[](jive::output * arg) {
					return typeid(arg->node()->operation()) == typeid(concat_op);
				});
			if (can_flatten) {
				return false;
			}
		}
		
		if (get_reducible()) {
			bool reducible = base::detail::pairwise_test_reduce(
				arguments,
				concat_test_reduce_arg_pair);
			if (reducible) {
				return false;
			}
		}
		
		return node_normal_form::operands_are_normalized(op, arguments);
	}

	virtual std::vector<jive::output *>
	normalized_create(
		const jive::operation & op,
		const std::vector<jive::output *> & arguments) const override
	{
		std::vector<jive::output *> new_args;

		/* possibly expand associative */
		if (get_mutable() && get_flatten()) {
			new_args = base::detail::associative_flatten(
				arguments,
				[](jive::output * arg) {
					// FIXME: switch to comparing operator, not just typeid, after
					// converting "concat" to not be a binary operator anymore
					return typeid(arg->node()->operation()) == typeid(concat_op);
				});
		} else {
			new_args = arguments;
		}

		if (get_mutable() && get_reducible()) {
			new_args = base::detail::pairwise_reduce(
				std::move(new_args),
				concat_reduce_arg_pair);
			if (new_args.size() == 1) {
				return std::move(new_args);
			}
		}

		concat_op new_op(types_from_arguments(new_args));
		return node_normal_form::normalized_create(new_op, new_args);
	}

	virtual void
	set_reducible(bool enable)
	{
		if (get_reducible() == enable) {
			return;
		}

		children_set<concat_normal_form, &concat_normal_form::set_reducible>(enable);

		enable_reducible_ = enable;
		if (get_mutable() && enable) {
			jive_graph_mark_denormalized(graph());
		}
	}
	inline bool
	get_reducible() const noexcept { return enable_reducible_; }

	virtual void
	set_flatten(bool enable)
	{
		if (get_flatten() == enable) {
			return;
		}

		children_set<concat_normal_form, &concat_normal_form::set_flatten>(enable);

		enable_flatten_ = enable;
		if (get_mutable() && enable) {
			jive_graph_mark_denormalized(graph());
		}
	}
	inline bool
	get_flatten() const noexcept { return enable_flatten_; }

private:
	bool enable_reducible_;
	bool enable_flatten_;
};

concat_normal_form::~concat_normal_form() noexcept
{
}

static node_normal_form *
get_default_normal_form(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive_graph * graph)
{
	return new concat_normal_form(parent, graph);
}

static void  __attribute__((constructor))
register_node_normal_form(void)
{
	jive::node_normal_form::register_factory(
		typeid(jive::bits::concat_op), get_default_normal_form);
}

type
concat_op::aggregate_arguments(const std::vector<type>& argument_types) noexcept
{
	size_t total = 0;
	for (const type & t : argument_types) {
		total += t.nbits();
	}
	return type(total);
}

concat_op::~concat_op() noexcept
{
}

bool
concat_op::operator==(const operation & other) const noexcept
{
	const concat_op * op = dynamic_cast<const concat_op *>(&other);
	return op && op->argument_types_ == argument_types_;
}

size_t
concat_op::narguments() const noexcept
{
	return argument_types_.size();
}

const jive::base::type &
concat_op::argument_type(size_t index) const noexcept
{
	return argument_types_[index];
}

size_t
concat_op::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
concat_op::result_type(size_t index) const noexcept
{
	return result_type_;
}
jive_binop_reduction_path_t
concat_op::can_reduce_operand_pair(
	const jive::output * arg1,
	const jive::output * arg2) const noexcept
{
	const constant_op * arg1_constant = dynamic_cast<const constant_op *>(
		&arg1->node()->operation());
	const constant_op * arg2_constant = dynamic_cast<const constant_op *>(
		&arg2->node()->operation());

	if (arg1_constant && arg2_constant) {
		return jive_binop_reduction_constants;
	}

	const slice_op * arg1_slice = dynamic_cast<const slice_op *>(
		&arg1->node()->operation());
	const slice_op * arg2_slice = dynamic_cast<const slice_op *>(
		&arg2->node()->operation());

	if (arg1_slice && arg2_slice){
		jive::output * origin1 = arg1->node()->inputs[0]->origin();
		jive::output * origin2 = arg2->node()->inputs[0]->origin();

		if (origin1 == origin2 && arg1_slice->high() == arg2_slice->low()) {
			return jive_binop_reduction_merge;
		}

		/* FIXME: support sign bit */
	}

	return jive_binop_reduction_none;
}

jive::output *
concat_op::reduce_operand_pair(
	jive_binop_reduction_path_t path,
	jive::output * arg1,
	jive::output * arg2) const
{
	jive_graph * graph = arg1->node()->graph;

	if (path == jive_binop_reduction_constants) {
		const constant_op & arg1_constant = static_cast<const constant_op &>(
			arg1->node()->operation());
		const constant_op & arg2_constant = static_cast<const constant_op &>(
			arg2->node()->operation());

		size_t nbits = arg1_constant.value().nbits() + arg2_constant.value().nbits();
		char bits[nbits];
		memcpy(bits, &arg1_constant.value()[0], arg1_constant.value().nbits());
		memcpy(
			bits + arg1_constant.value().nbits(),
			&arg2_constant.value()[0],
			arg2_constant.value().nbits());

		return jive_bitconstant(graph, nbits, bits);
	}

	if (path == jive_binop_reduction_merge) {
		const slice_op * arg1_slice = static_cast<const slice_op *>(
			&arg1->node()->operation());
		const slice_op * arg2_slice = static_cast<const slice_op *>(
			&arg2->node()->operation());

		jive::output * origin1 = arg1->node()->inputs[0]->origin();

		return jive_bitslice(origin1, arg1_slice->low(), arg2_slice->high());

		/* FIXME: support sign bit */
	}

	return NULL;
}

jive_binary_operation_flags
concat_op::flags() const noexcept
{
	return jive_binary_operation_associative;
}

std::string
concat_op::debug_string() const
{
	return "BITCONCAT";
}

std::unique_ptr<jive::operation>
concat_op::copy() const
{
	return std::unique_ptr<jive::operation>(new concat_op(*this));
}

}
}
