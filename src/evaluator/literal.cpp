/*
 * Copyright 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/addresstype.hpp>
#include <jive/evaluator/literal.hpp>
#include <jive/util/ptr-collection.hpp>

namespace jive {
namespace eval {

/* literal */

literal::~literal() noexcept
{}

/* bitliteral */

bitliteral::~bitliteral() noexcept
{}

const jive::type &
bitliteral::type() const noexcept
{
	return type_;
}

std::unique_ptr<literal>
bitliteral::copy() const
{
	return std::unique_ptr<literal>(new bitliteral(*this));
}

/* ctlliteral */

ctlliteral::~ctlliteral() noexcept
{}

const jive::type &
ctlliteral::type() const noexcept
{
	return type_;
}

std::unique_ptr<literal>
ctlliteral::copy() const
{
	return std::unique_ptr<literal>(new ctlliteral(*this));
}

/* fctliteral */

fctliteral::~fctliteral() noexcept
{}

fctliteral::fctliteral(
	const std::vector<std::unique_ptr<const literal>> & arguments,
	const std::vector<std::unique_ptr<const literal>> & results)
	: arguments_(detail::unique_ptr_vector_copy(arguments))
	, results_(detail::unique_ptr_vector_copy(results))
{
	std::vector<std::unique_ptr<jive::type>> argument_types;
	for (size_t n = 0; n < arguments.size(); n++)
		argument_types.emplace_back(arguments[n]->type().copy());

	std::vector<std::unique_ptr<jive::type>> result_types;
	for (size_t n = 0; n < results.size(); n++)
		result_types.emplace_back(results[n]->type().copy());

	type_ = std::make_unique<fcttype>(argument_types, result_types);
}

fctliteral::fctliteral(
	const std::vector<const literal*> & arguments,
	const std::vector<const literal*> & results)
	: arguments_(detail::unique_ptr_vector_copy(arguments))
	, results_(detail::unique_ptr_vector_copy(results))
{
	std::vector<std::unique_ptr<jive::type>> argument_types;
	for (auto argument : arguments)
		argument_types.emplace_back(argument->type().copy());

	std::vector<std::unique_ptr<jive::type>> result_types;
	for (auto result : results)
		result_types.emplace_back(result->type().copy());

	type_ = std::make_unique<fcttype>(argument_types, result_types);
}

fctliteral::fctliteral(const fctliteral & other)
	: type_(other.type_->copy())
{
	for (size_t n = 0; n < other.arguments_.size(); n++)
		arguments_.emplace_back(other.arguments_[n]->copy());

	for (size_t n = 0; n < other.results_.size(); n++)
		results_.emplace_back(other.results_[n]->copy());
}

fctliteral &
fctliteral::operator=(const fctliteral & other)
{
	arguments_.clear();
	for (size_t n = 0; n < other.arguments_.size(); n++)
		arguments_.emplace_back(other.arguments_[n]->copy());

	results_.clear();
	for (size_t n = 0; n < other.results_.size(); n++)
		results_.emplace_back(other.results_[n]->copy());

	type_ = other.type_->copy();
	return *this;
}

const jive::type &
fctliteral::type() const noexcept
{
	return *type_;
}

std::unique_ptr<literal>
fctliteral::copy() const
{
	return std::unique_ptr<literal>(new fctliteral(*this));
}

/* memliteral */

memliteral::~memliteral() noexcept
{}

const jive::type &
memliteral::type() const noexcept
{
	return jive::memtype::instance();
}

std::unique_ptr<literal>
memliteral::copy() const
{
	return std::unique_ptr<literal>(new memliteral());
}

}
}
