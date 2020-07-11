/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 2018 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/addresstype.hpp>
#include <jive/arch/load.hpp>
#include <jive/types/bitstring.hpp>
#include <jive/types/union.hpp>

namespace {

typedef std::unordered_set<std::unique_ptr<jive::unndeclaration>> declarationset;
typedef std::unordered_map<const jive::graph*, declarationset> declarationmap;

declarationmap &
map()
{
	static declarationmap map;
	return map;
}

void
register_declaration(
	const jive::graph * graph,
	std::unique_ptr<jive::unndeclaration> dcl)
{
	auto & m = map();
	if (m.find(graph) == m.end())
		m[graph] = declarationset();

	m[graph].insert(std::move(dcl));
}

}

namespace jive {

/* union declaration */

void
unregister_unndeclarations(const jive::graph * graph)
{
	map().erase(graph);
}

unndeclaration *
unndeclaration::create(const jive::graph * graph)
{
	std::unique_ptr<jive::unndeclaration> dcl(new jive::unndeclaration());
	auto ptr = dcl.get();

	register_declaration(graph, std::move(dcl));
	return ptr;
}

/* union type */

unntype::~unntype() noexcept
{}

std::string
unntype::debug_string() const
{
	return "unn";
}

bool
unntype::operator==(const jive::type & other) const noexcept
{
	auto type = dynamic_cast<const jive::unntype*>(&other);
	return type
	    && declaration() == type->declaration();
}

std::unique_ptr<jive::type>
unntype::copy() const
{
	return std::unique_ptr<jive::type>(new unntype(*this));
}

/* choose operator */

choose_op::~choose_op() noexcept
{}

bool
choose_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const choose_op*>(&other);
	return op
	    && op->option() == option()
	    && op->declaration() == declaration();
}

std::string
choose_op::debug_string() const
{
	return detail::strfmt("CHOOSE(", option(), ")");
}

jive_unop_reduction_path_t
choose_op::can_reduce_operand(
	const jive::output * arg) const noexcept
{
	if (is<unify_op>(node_output::node(arg)))
		return jive_unop_reduction_inverse;

	return jive_unop_reduction_none;
}

jive::output *
choose_op::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	auto node = static_cast<node_output*>(arg)->node();

	if (path == jive_unop_reduction_inverse)
		return node->input(0)->origin();

	return nullptr;
}

std::unique_ptr<jive::operation>
choose_op::copy() const
{
	return std::unique_ptr<jive::operation>(new choose_op(*this));
}

/* unify operator */

unify_op::~unify_op() noexcept
{}

bool
unify_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const unify_op*>(&other);
	return op
	    && op->option() == option()
	    && op->declaration() == declaration();
}

std::string
unify_op::debug_string() const
{
	return detail::strfmt("UNIFY(", option(), ")");
}

jive_unop_reduction_path_t
unify_op::can_reduce_operand(
	const jive::output * arg) const noexcept
{
	return jive_unop_reduction_none;
}

jive::output *
unify_op::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	return nullptr;
}

std::unique_ptr<jive::operation>
unify_op::copy() const
{
	return std::unique_ptr<jive::operation>(new unify_op(*this));
}

}

jive::output *
jive_unify_create(
	const jive::unndeclaration * decl,
	size_t option,
	jive::output * const argument)
{
	jive::unify_op op(decl, option);
	return jive::simple_node::create_normalized(argument->region(), op, {argument})[0];
}

/* empty unify operation */

namespace jive {

empty_unify_op::~empty_unify_op() noexcept
{}

bool
empty_unify_op::operator==(const jive::operation & other) const noexcept
{
	auto op = dynamic_cast<const empty_unify_op*>(&other);
	return op && op->declaration() == declaration();
}

std::string
empty_unify_op::debug_string() const
{
	return "UNIFY";
}

std::unique_ptr<jive::operation>
empty_unify_op::copy() const
{
	return std::unique_ptr<jive::operation>(new empty_unify_op(*this));
}

}

jive::output *
jive_empty_unify_create(
	jive::region * region,
	const jive::unndeclaration * decl)
{
	jive::empty_unify_op op(decl);
	return jive::simple_node::create_normalized(region, op, {})[0];
}
