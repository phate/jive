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

/* function_type inheritable members */

jive_function_type::~jive_function_type() noexcept {}

jive_function_type::jive_function_type(size_t narguments, const jive_type ** argument_types,
	size_t nreturns, const jive_type ** return_types)
	: jive_value_type()
{
	size_t i;
	for (i = 0; i < narguments; i++)
		argument_types_.push_back(std::unique_ptr<jive_type>(jive_type_copy(argument_types[i])));

	for (i = 0; i < nreturns; i++)
		return_types_.push_back(std::unique_ptr<jive_type>(jive_type_copy(return_types[i])));
}

jive_function_type::jive_function_type(
	const std::vector<std::unique_ptr<jive_type>> & argument_types,
	const std::vector<std::unique_ptr<jive_type>> & return_types)
	: jive_value_type()
{
	for (size_t i = 0; i < argument_types.size(); i++)
		argument_types_.push_back(std::unique_ptr<jive_type>(jive_type_copy(argument_types[i].get())));

	for (size_t i = 0; i < return_types.size(); i++)
		return_types_.push_back(std::unique_ptr<jive_type>(jive_type_copy(return_types[i].get())));
}

jive_function_type::jive_function_type(const jive_function_type & rhs)
	: jive_value_type()
{
	size_t i;
	for (i = 0; i < rhs.narguments(); i++)
		argument_types_.push_back(std::unique_ptr<jive_type>(jive_type_copy(rhs.argument_type(i))));

	for (i = 0; i < rhs.nreturns(); i++)
		return_types_.push_back(std::unique_ptr<jive_type>(jive_type_copy(rhs.return_type(i))));
}

jive_function_type::jive_function_type(jive_function_type && other) noexcept
	: jive_value_type()
	, return_types_(std::move(other.return_types_))
	, argument_types_(std::move(other.argument_types_))
{
}

void
jive_function_type::label(jive_buffer & buffer) const
{
	jive_buffer_putstr(&buffer, "fct");
}

bool
jive_function_type::operator==(const jive_type & _other) const noexcept
{
	const jive_function_type * other = dynamic_cast<const jive_function_type*>(&_other);
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

jive_function_type *
jive_function_type::copy() const
{
 return new jive_function_type(argument_types_, return_types_);
}

jive_input *
jive_function_type::create_input(jive_node * node, size_t index, jive_output * origin) const
{
	return new jive_function_input(*this, node, index, origin);
}

jive_output *
jive_function_type::create_output(jive_node * node, size_t index) const
{
	return new jive_function_output(*this, node, index);
}

jive_gate *
jive_function_type::create_gate(jive_graph * graph, const char * name) const
{
	return new jive_function_gate(*this, graph, name);
}

/* function_input inheritable members */

jive_function_input::jive_function_input(size_t narguments, const jive_type ** argument_types,
	size_t nreturns, const jive_type ** return_types, struct jive_node * node, size_t index,
	jive_output * origin)
	: jive_value_input(node, index, origin)
	, type_(narguments, argument_types, nreturns, return_types)
{}

jive_function_input::jive_function_input(const jive_function_type & type, jive_node * node,
	size_t index, jive_output * origin)
	: jive_value_input(node, index, origin)
	, type_(type)
{}

jive_function_input::~jive_function_input() noexcept {}

/* function_output inheritable members */

jive_function_output::jive_function_output(size_t narguments, const jive_type ** argument_types,
	size_t nreturns, const jive_type ** return_types, jive_node * node, size_t index)
	: jive_value_output(node, index)
	, type_(narguments, argument_types, nreturns, return_types)
{}

jive_function_output::jive_function_output(const jive_function_type & type, jive_node * node,
	size_t index)
	: jive_value_output(node, index)
	, type_(type)
{}

jive_function_output::~jive_function_output() noexcept {}

/* function_gate inheritable members */

jive_function_gate::jive_function_gate(size_t narguments, const jive_type ** argument_types,
	size_t nreturns, const jive_type ** return_types, jive_graph * graph, const char name[])
	: jive_value_gate(graph, name)
	, type_(narguments, argument_types, nreturns, return_types)
{}

jive_function_gate::jive_function_gate(const jive_function_type & type, jive_graph * graph,
	const char name[])
	: jive_value_gate(graph, name)
	, type_(type)
{}

jive_function_gate::~jive_function_gate() noexcept {}
