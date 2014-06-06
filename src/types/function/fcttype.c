/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/function/fcttype.h>

#include <string.h>
#include <stdio.h>

#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

namespace jive {
namespace fct {

/* type */

type::~type() noexcept {}

type::type(size_t narguments, const jive::base::type ** argument_types, size_t nreturns,
	const jive::base::type ** return_types)
	: jive::value::type()
{
	size_t i;
	for (i = 0; i < narguments; i++)
		argument_types_.push_back(std::unique_ptr<jive::base::type>(argument_types[i]->copy()));

	for (i = 0; i < nreturns; i++)
		return_types_.push_back(std::unique_ptr<jive::base::type>(return_types[i]->copy()));
}

type::type(
	const std::vector<std::unique_ptr<jive::base::type>> & argument_types,
	const std::vector<std::unique_ptr<jive::base::type>> & return_types)
	: jive::value::type()
{
	for (size_t i = 0; i < argument_types.size(); i++)
		argument_types_.push_back(std::unique_ptr<jive::base::type>(argument_types[i]->copy()));

	for (size_t i = 0; i < return_types.size(); i++)
		return_types_.push_back(std::unique_ptr<jive::base::type>(return_types[i]->copy()));
}

type::type(const jive::fct::type & rhs)
	: jive::value::type()
{
	size_t i;
	for (i = 0; i < rhs.narguments(); i++)
		argument_types_.push_back(std::unique_ptr<jive::base::type>(rhs.argument_type(i)->copy()));

	for (i = 0; i < rhs.nreturns(); i++)
		return_types_.push_back(std::unique_ptr<jive::base::type>(rhs.return_type(i)->copy()));
}

type::type(jive::fct::type && other) noexcept
	: jive::value::type()
	, return_types_(std::move(other.return_types_))
	, argument_types_(std::move(other.argument_types_))
{
}

void
type::label(jive_buffer & buffer) const
{
	jive_buffer_putstr(&buffer, "fct");
}

bool
type::operator==(const jive::base::type & _other) const noexcept
{
	const jive::fct::type * other = dynamic_cast<const jive::fct::type*>(&_other);
	if (other == nullptr)
		return false;

	if (this->nreturns() != other->nreturns())
		return false;

	if (this->narguments() != other->narguments())
		return false;

	for (size_t i = 0; i < this->nreturns(); i++){
		if (*this->return_type(i) != *other->return_type(i))
			return false;
	}

	for (size_t i = 0; i < this->narguments(); i++){
		if (*this->argument_type(i) != *other->argument_type(i))
			return false;
	}

	return true;
}

jive::fct::type *
type::copy() const
{
 return new jive::fct::type(argument_types_, return_types_);
}

jive_input *
type::create_input(jive_node * node, size_t index, jive_output * origin) const
{
	return new jive::fct::input(*this, node, index, origin);
}

jive_output *
type::create_output(jive_node * node, size_t index) const
{
	return new jive::fct::output(*this, node, index);
}

jive_gate *
type::create_gate(jive_graph * graph, const char * name) const
{
	return new jive::fct::gate(*this, graph, name);
}

/* input */

input::input(size_t narguments, const jive::base::type ** argument_types,
	size_t nreturns, const jive::base::type ** return_types, struct jive_node * node, size_t index,
	jive_output * origin)
	: jive::value::input(node, index, origin)
	, type_(narguments, argument_types, nreturns, return_types)
{}

input::input(const jive::fct::type & type, jive_node * node,
	size_t index, jive_output * origin)
	: jive::value::input(node, index, origin)
	, type_(type)
{}

input::~input() noexcept {}

/* output */

output::output(size_t narguments, const jive::base::type ** argument_types,
	size_t nreturns, const jive::base::type ** return_types, jive_node * node, size_t index)
	: jive::value::output(node, index)
	, type_(narguments, argument_types, nreturns, return_types)
{}

output::output(const jive::fct::type & type, jive_node * node, size_t index)
	: jive::value::output(node, index)
	, type_(type)
{}

output::~output() noexcept {}

/* gate */

gate::gate(size_t narguments, const jive::base::type ** argument_types,
	size_t nreturns, const jive::base::type ** return_types, jive_graph * graph, const char name[])
	: jive::value::gate(graph, name)
	, type_(narguments, argument_types, nreturns, return_types)
{}

gate::gate(const jive::fct::type & type, jive_graph * graph, const char name[])
	: jive::value::gate(graph, name)
	, type_(type)
{}

gate::~gate() noexcept {}

}
}
