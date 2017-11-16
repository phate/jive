/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/concat.h>

#include <string.h>

#include <jive/common.h>

#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/reduction-helpers.h>
#include <jive/rvsdg/region.h>
#include <jive/rvsdg/simple-normal-form.h>
#include <jive/types/bitstring/constant.h>
#include <jive/types/bitstring/slice.h>
#include <jive/types/bitstring/type.h>

jive::output *
jive_bitconcat(const std::vector<jive::output*> & operands)
{
	std::vector<jive::bits::type> types;
	for (const auto operand : operands)
		types.push_back(dynamic_cast<const jive::bits::type &>(operand->type()));

	jive::region* region = operands[0]->region();

	jive::bits::concat_op op(std::move(types));
	return jive::create_normalized(region, op, {operands.begin(), operands.end()})[0];
}

namespace jive {
namespace bits {

namespace {

jive::output *
concat_reduce_arg_pair(jive::output * arg1, jive::output * arg2)
{
	if (!arg1->node() || !arg2->node())
		return nullptr;

	auto arg1_constant = dynamic_cast<const constant_op*>(&arg1->node()->operation());
	auto arg2_constant = dynamic_cast<const constant_op*>(&arg2->node()->operation());
	if (arg1_constant && arg2_constant) {
		size_t nbits = arg1_constant->value().nbits() + arg2_constant->value().nbits();
		char bits[nbits];
		memcpy(bits, &arg1_constant->value()[0], arg1_constant->value().nbits());
		memcpy(
			bits + arg1_constant->value().nbits(),
			&arg2_constant->value()[0],
			arg2_constant->value().nbits());

		std::string s(bits, nbits);
		return create_bitconstant(arg1->node()->region(), s.c_str());
	}

	auto arg1_slice = dynamic_cast<const slice_op*>(&arg1->node()->operation());
	auto arg2_slice = dynamic_cast<const slice_op*>(&arg2->node()->operation());
	if (arg1_slice && arg2_slice && arg1_slice->high() == arg2_slice->low() &&
		arg1->node()->input(0)->origin() == arg2->node()->input(0)->origin()) {
		/* FIXME: support sign bit */
		return jive_bitslice(arg1->node()->input(0)->origin(), arg1_slice->low(), arg2_slice->high());
	}

	return nullptr;
}

std::vector<jive::bits::type>
types_from_arguments(const std::vector<jive::output*> & args)
{
	std::vector<type> types;
	for (const auto arg : args) {
		types.push_back(static_cast<const type &>(arg->type()));
	}
	return types;
}

}

class concat_normal_form final : public simple_normal_form {
public:
	virtual
	~concat_normal_form() noexcept;

	concat_normal_form(
		jive::node_normal_form * parent,
		jive::graph * graph)
		: simple_normal_form(typeid(concat_op), parent, graph)
		, enable_reducible_(true)
		, enable_flatten_(true)
	{
	}

	virtual bool
	normalize_node(jive::node * node) const override
	{
		if (!get_mutable()) {
			return true;
		}

		auto args = operands(node);
		std::vector<jive::output*> new_args;

		/* possibly expand associative */
		if (get_flatten()) {
			new_args = base::detail::associative_flatten(
				args,
				[](jive::output * arg) {
					// FIXME: switch to comparing operator, not just typeid, after
					// converting "concat" to not be a binary operator anymore
					return arg->node() && typeid(arg->node()->operation()) == typeid(concat_op);
				});
		} else {
			new_args = args;
		}

		if (get_reducible()) {
			new_args = base::detail::pairwise_reduce(
				std::move(new_args),
				concat_reduce_arg_pair);

			if (new_args.size() == 1) {
				node->output(0)->replace(new_args[0]);
				node->region()->remove_node(node);
				return false;
			}
		}
		
		bool changes = (args != new_args);

		if (changes) {
			jive::node * new_node = nullptr;
			concat_op op(types_from_arguments(new_args));

			if (get_cse()) {
				jive_node_cse(node->region(), op, new_args);
			}

			JIVE_DEBUG_ASSERT(new_args.size() >= 2);
			if (!new_node) {
				new_node = node->region()->add_simple_node(op, new_args);
			}

			if (new_node != node) {
				node->output(0)->replace(new_node->output(0));
				node->region()->remove_node(node);
				return false;
			}
		}

		return true;
	}

	virtual std::vector<jive::output*>
	normalized_create(
		jive::region * region,
		const jive::simple_op & op,
		const std::vector<jive::output*> & arguments) const override
	{
		std::vector<jive::output*> new_args;

		/* possibly expand associative */
		if (get_mutable() && get_flatten()) {
			new_args = base::detail::associative_flatten(
				arguments,
				[](jive::output * arg) {
					// FIXME: switch to comparing operator, not just typeid, after
					// converting "concat" to not be a binary operator anymore
					return arg->node() && typeid(arg->node()->operation()) == typeid(concat_op);
				});
		} else {
			new_args = arguments;
		}

		if (get_mutable() && get_reducible()) {
			new_args = base::detail::pairwise_reduce(
				std::move(new_args),
				concat_reduce_arg_pair);
			if (new_args.size() == 1)
				return std::move(new_args);
		}

		concat_op new_op(types_from_arguments(new_args));
		return simple_normal_form::normalized_create(region, new_op, new_args);
	}

	virtual void
	set_reducible(bool enable)
	{
		if (get_reducible() == enable) {
			return;
		}

		children_set<concat_normal_form, &concat_normal_form::set_reducible>(enable);

		enable_reducible_ = enable;
		if (get_mutable() && enable)
			graph()->mark_denormalized();
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
		if (get_mutable() && enable)
			graph()->mark_denormalized();
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
	jive::graph * graph)
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
	auto op = dynamic_cast<const concat_op*>(&other);
	return op && op->result_ == result_ && op->arguments_ == arguments_;
}

size_t
concat_op::narguments() const noexcept
{
	return arguments_.size();
}

const jive::port &
concat_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return arguments_[index];
}

size_t
concat_op::nresults() const noexcept
{
	return 1;
}

const jive::port &
concat_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return result_;
}

jive_binop_reduction_path_t
concat_op::can_reduce_operand_pair(
	const jive::output * arg1,
	const jive::output * arg2) const noexcept
{
	if (!arg1->node() || !arg2->node())
		return jive_binop_reduction_none;

	auto arg1_constant = dynamic_cast<const constant_op*>(&arg1->node()->operation());
	auto arg2_constant = dynamic_cast<const constant_op*>(&arg2->node()->operation());

	if (arg1_constant && arg2_constant) {
		return jive_binop_reduction_constants;
	}

	auto arg1_slice = dynamic_cast<const slice_op*>(&arg1->node()->operation());
	auto arg2_slice = dynamic_cast<const slice_op*>(&arg2->node()->operation());

	if (arg1_slice && arg2_slice){
		auto origin1 = arg1->node()->input(0)->origin();
		auto origin2 = arg2->node()->input(0)->origin();

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
	if (path == jive_binop_reduction_constants) {
		auto arg1_constant = static_cast<const constant_op &>(arg1->node()->operation());
		auto arg2_constant = static_cast<const constant_op &>(arg2->node()->operation());

		size_t nbits = arg1_constant.value().nbits() + arg2_constant.value().nbits();
		char bits[nbits];
		memcpy(bits, &arg1_constant.value()[0], arg1_constant.value().nbits());
		memcpy(
			bits + arg1_constant.value().nbits(),
			&arg2_constant.value()[0],
			arg2_constant.value().nbits());

		return create_bitconstant(arg1->region(), bits);
	}

	if (path == jive_binop_reduction_merge) {
		auto arg1_slice = static_cast<const slice_op *>(&arg1->node()->operation());
		auto arg2_slice = static_cast<const slice_op *>(&arg2->node()->operation());
		return jive_bitslice(arg1->node()->input(0)->origin(), arg1_slice->low(), arg2_slice->high());

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
