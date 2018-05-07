/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/node.h>
#include <jive/types/function/fcttype.h>

#include <stdio.h>
#include <string.h>

namespace jive {

/* function type */

fcttype::~fcttype() noexcept
{}

fcttype::fcttype(
	const std::vector<const jive::type*> & argument_types,
	const std::vector<const jive::type*> & result_types)
: jive::valuetype()
{
	for (const auto & type : argument_types)
		argument_types_.push_back(std::unique_ptr<jive::type>(type->copy()));

	for (const auto & type : result_types)
		result_types_.push_back(std::unique_ptr<jive::type>(type->copy()));
}

fcttype::fcttype(
	const std::vector<std::unique_ptr<jive::type>> & argument_types,
	const std::vector<std::unique_ptr<jive::type>> & result_types)
: jive::valuetype()
{
	for (size_t i = 0; i < argument_types.size(); i++)
		argument_types_.push_back(std::unique_ptr<jive::type>(argument_types[i]->copy()));

	for (size_t i = 0; i < result_types.size(); i++)
		result_types_.push_back(std::unique_ptr<jive::type>(result_types[i]->copy()));
}

fcttype::fcttype(const jive::fcttype & rhs)
: jive::valuetype(rhs)
{
	for (size_t i = 0; i < rhs.narguments(); i++)
		argument_types_.push_back(std::unique_ptr<jive::type>(rhs.argument_type(i).copy()));

	for (size_t i = 0; i < rhs.nresults(); i++)
		result_types_.push_back(std::unique_ptr<jive::type>(rhs.result_type(i).copy()));
}

fcttype::fcttype(jive::fcttype && other)
: jive::valuetype(other)
, result_types_(std::move(other.result_types_))
, argument_types_(std::move(other.argument_types_))
{}

std::string
fcttype::debug_string() const
{
	return "fct";
}

bool
fcttype::operator==(const jive::type & _other) const noexcept
{
	auto other = dynamic_cast<const jive::fcttype*>(&_other);
	if (other == nullptr)
		return false;

	if (this->nresults() != other->nresults())
		return false;

	if (this->narguments() != other->narguments())
		return false;

	for (size_t i = 0; i < this->nresults(); i++){
		if (this->result_type(i) != other->result_type(i))
			return false;
	}

	for (size_t i = 0; i < this->narguments(); i++){
		if (this->argument_type(i) != other->argument_type(i))
			return false;
	}

	return true;
}

std::unique_ptr<jive::type>
fcttype::copy() const
{
 return std::unique_ptr<jive::type>(new fcttype(*this));
}

jive::fcttype &
fcttype::operator=(const jive::fcttype & rhs)
{
	result_types_.clear();
	argument_types_.clear();

	for (size_t i = 0; i < rhs.narguments(); i++)
		argument_types_.push_back(std::unique_ptr<jive::type>(rhs.argument_type(i).copy()));

	for (size_t i = 0; i < rhs.nresults(); i++)
		result_types_.push_back(std::unique_ptr<jive::type>(rhs.result_type(i).copy()));

	return *this;
}

jive::fcttype &
fcttype::operator=(jive::fcttype && rhs)
{
	result_types_ = std::move(rhs.result_types_);
	argument_types_ = std::move(rhs.argument_types_);
	return *this;
}

}
