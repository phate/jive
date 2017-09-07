/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FUNCTION_FCTLAMBDA_H
#define JIVE_TYPES_FUNCTION_FCTLAMBDA_H

#include <memory>
#include <vector>

#include <jive/types/function/fcttype.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/simple_node.h>
#include <jive/vsdg/structural_node.h>

namespace jive {
namespace fct {

class lambda_op final : public structural_op {
public:
	virtual
	~lambda_op() noexcept;

	inline
	lambda_op(const lambda_op & other) = default;

	inline
	lambda_op(lambda_op && other) = default;

	inline
	lambda_op(jive::fct::type function_type) noexcept
		: function_type_(std::move(function_type))
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual size_t
	narguments() const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual std::string
	debug_string() const override;

	inline const jive::fct::type &
	function_type() const noexcept
	{
		return function_type_;
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	jive::fct::type function_type_;
};

static inline bool
is_lambda_op(const jive::operation & op) noexcept
{
	return dynamic_cast<const jive::fct::lambda_op*>(&op) != nullptr;
}

}
}

bool
jive_lambda_is_self_recursive(const jive::node * self);

void
jive_inline_lambda_apply(jive::node * apply_node);

namespace jive {

class argument;

class lambda final {
public:
	inline
	lambda(jive::structural_node * node)
	: node_(node)
	{
		if (!fct::is_lambda_op(node->operation()))
			throw jive::compiler_error("Expected lambda node.");
	}

private:
	class dependency_iterator {
	public:
		inline constexpr
		dependency_iterator(jive::input * input) noexcept
		: input_(input)
		{}

		inline const dependency_iterator &
		operator++() noexcept
		{
			auto node = input_->node();
			auto index = input_->index();
			input_ = (index == node->ninputs()-1) ? nullptr : node->input(index+1);
			return *this;
		}

		inline const dependency_iterator
		operator++(int) noexcept
		{
			dependency_iterator it(*this);
			++(*this);
			return it;
		}

		inline bool
		operator==(const dependency_iterator & other) const noexcept
		{
			return input_ == other.input_;
		}

		inline bool
		operator!=(const dependency_iterator & other) const noexcept
		{
			return !(*this == other);
		}

		inline jive::input *
		operator*() noexcept
		{
			return input_;
		}

	private:
		jive::input * input_;
	};

public:
	inline jive::structural_node *
	node() const noexcept
	{
		return node_;
	}

	inline jive::region *
	subregion() const noexcept
	{
		return node_->subregion(0);
	}

	inline lambda::dependency_iterator
	begin() const
	{
		auto argument = subregion()->argument(0);
		while (argument->input() == nullptr && argument != nullptr)
			argument = subregion()->argument(argument->index()+1);

		return dependency_iterator(argument->input());
	}

	inline lambda::dependency_iterator
	end() const
	{
		return dependency_iterator(nullptr);
	}

	inline jive::argument *
	add_dependency(jive::output * origin)
	{
		auto input = node_->add_input(origin->type(), origin);
		return subregion()->add_argument(input, origin->type());
	}

private:
	jive::structural_node * node_;
};

class lambda_builder final {
public:
	inline std::vector<jive::output*>
	begin(jive::region * parent, jive::fct::type type)
	{
		std::vector<jive::output*> arguments;

		if (lambda_) {
			auto argument = lambda_->subregion()->argument(0);
			while (argument->input() == nullptr && argument != nullptr) {
				arguments.push_back(argument);
				argument = lambda_->subregion()->argument(argument->index()+1);
			}
			return arguments;
		}

		auto node = parent->add_structural_node(jive::fct::lambda_op(std::move(type)), 1);
		lambda_ = std::make_unique<jive::lambda>(node);

		for (size_t n = 0; n < type.narguments(); n++)
			arguments.push_back(lambda_->subregion()->add_argument(nullptr, type.argument_type(n)));

		return arguments;
	}

	inline jive::region *
	subregion() const noexcept
	{
		return lambda_ ? lambda_->subregion() : nullptr;
	}

	inline jive::output *
	add_dependency(jive::output * value)
	{
		return lambda_ ? lambda_->add_dependency(value) : nullptr;
	}

	inline std::unique_ptr<jive::lambda>
	end(const std::vector<jive::output*> & results)
	{
		if (!lambda_)
			return nullptr;

		const auto & ftype = static_cast<const fct::lambda_op*>(
			&lambda_->node()->operation())->function_type();
		if (results.size() != ftype.nresults())
			throw jive::compiler_error("Incorrect number of results.");

		for (size_t n = 0; n < results.size(); n++)
			lambda_->node()->subregion(0)->add_result(results[n], nullptr, ftype.result_type(n));
		lambda_->node()->add_output(ftype);

		return std::move(lambda_);
	}

private:
	std::unique_ptr<jive::lambda> lambda_;
};

}

#endif
