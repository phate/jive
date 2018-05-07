/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FUNCTION_FCTLAMBDA_H
#define JIVE_TYPES_FUNCTION_FCTLAMBDA_H

#include <memory>
#include <vector>

#include <jive/rvsdg/region.h>
#include <jive/rvsdg/simple-node.h>
#include <jive/rvsdg/structural-node.h>
#include <jive/types/function/fcttype.h>

namespace jive {

class lambda_op final : public structural_op {
public:
	virtual
	~lambda_op() noexcept;

	inline
	lambda_op(const lambda_op & other) = default;

	inline
	lambda_op(lambda_op && other) = default;

	inline
	lambda_op(jive::fcttype function_type) noexcept
		: function_type_(std::move(function_type))
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual std::string
	debug_string() const override;

	inline const jive::fcttype &
	function_type() const noexcept
	{
		return function_type_;
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	fcttype function_type_;
};

static inline bool
is_lambda_op(const jive::operation & op) noexcept
{
	return dynamic_cast<const lambda_op*>(&op) != nullptr;
}

class argument;
class lambda_builder;

class lambda_node final : public jive::structural_node {
	friend lambda_builder;
public:
	virtual
	~lambda_node();

private:
	inline
	lambda_node(jive::region * parent, fcttype type)
	: jive::structural_node(jive::lambda_op(std::move(type)), parent, 1)
	{}

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

	static jive::lambda_node *
	create(jive::region * parent, fcttype type)
	{
		return new jive::lambda_node(parent, std::move(type));
	}

public:
	inline jive::region *
	subregion() const noexcept
	{
		return structural_node::subregion(0);
	}

	inline lambda_node::dependency_iterator
	begin() const
	{
		auto argument = subregion()->argument(0);
		while (argument->input() == nullptr && argument != nullptr)
			argument = subregion()->argument(argument->index()+1);

		return dependency_iterator(argument->input());
	}

	inline lambda_node::dependency_iterator
	end() const
	{
		return dependency_iterator(nullptr);
	}

	inline jive::argument *
	add_dependency(jive::output * origin)
	{
		auto input = add_input(origin->type(), origin);
		return subregion()->add_argument(input, origin->type());
	}

	inline const fcttype &
	function_type() const noexcept
	{
		return static_cast<const lambda_op*>(&operation())->function_type();
	}

	virtual jive::lambda_node *
	copy(jive::region * region, jive::substitution_map & smap) const override;
};

class lambda_builder final {
public:
	inline
	lambda_builder()
	: lambda_(nullptr)
	{}

	inline std::vector<jive::argument*>
	begin_lambda(jive::region * parent, fcttype type)
	{
		std::vector<jive::argument*> arguments;

		if (lambda_) {
			auto argument = lambda_->subregion()->argument(0);
			while (argument->input() == nullptr && argument != nullptr) {
				arguments.push_back(argument);
				argument = lambda_->subregion()->argument(argument->index()+1);
			}
			return arguments;
		}

		lambda_ = jive::lambda_node::create(parent, std::move(type));
		for (size_t n = 0; n < lambda_->function_type().narguments(); n++) {
			auto & argument_type = lambda_->function_type().argument_type(n);
			arguments.push_back(lambda_->subregion()->add_argument(nullptr, argument_type));
		}
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

	inline jive::lambda_node *
	end_lambda(const std::vector<jive::output*> & results)
	{
		if (!lambda_)
			return nullptr;

		const auto & ftype = lambda_->function_type();
		if (results.size() != ftype.nresults())
			throw jive::compiler_error("Incorrect number of results.");

		for (size_t n = 0; n < results.size(); n++)
			lambda_->subregion()->add_result(results[n], nullptr, ftype.result_type(n));
		lambda_->add_output(ftype);

		auto lambda = lambda_;
		lambda_ = nullptr;
		return lambda;
	}

private:
	jive::lambda_node * lambda_;
};

}

#endif
