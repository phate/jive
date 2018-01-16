/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 2018 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/addresstype.h>
#include <jive/arch/load.h>
#include <jive/types/bitstring.h>
#include <jive/types/union.h>

static constexpr jive_unop_reduction_path_t jive_choose_reduction_load = 128;

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
	    && option_ == op->option_
	    && unary_op::operator==(other);
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
	if (arg->node() && dynamic_cast<const unify_op*>(&arg->node()->operation()))
		return jive_unop_reduction_inverse;

	if (arg->node() && dynamic_cast<const load_op*>(&arg->node()->operation()))
		return jive_choose_reduction_load;

	return jive_unop_reduction_none;
}

jive::output *
choose_op::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	if (path == jive_unop_reduction_inverse) {
		return arg->node()->input(0)->origin();
	}

	if (path == jive_choose_reduction_load) {
		auto address = arg->node()->input(0)->origin();
		auto dcl = static_cast<const jive::unntype*>(&arg->node()->output(0)->type())->declaration();

		size_t nstates = arg->node()->ninputs()-1;
		std::vector<jive::output*> states;
		for (size_t n = 0; n < nstates; n++)
			states.push_back(arg->node()->input(n+1)->origin());

		if (dynamic_cast<const jive::addrtype*>(&address->type())) {
			return addrload_op::create(address, dcl->option(option()), states);
		} else {
			size_t nbits = static_cast<const bittype*>(&address->type())->nbits();
			return bitload_op::create(address, nbits, dcl->option(option()), states);
		}
	}

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
	    && option_ == op->option_
	    && unary_op::operator==(other);
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
	return jive::create_normalized(argument->region(), op, {argument})[0];
}

/* empty unify operation */

namespace jive {

empty_unify_op::~empty_unify_op() noexcept
{}

bool
empty_unify_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const empty_unify_op *>(&other);
	return op && op->port_ == port_;
}

std::string
empty_unify_op::debug_string() const
{
	return "UNIFY";
}

const jive::port &
empty_unify_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return port_;
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
	return jive::create_normalized(region, op, {})[0];
}
