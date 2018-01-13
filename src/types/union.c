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

namespace jive {
namespace unn {

/* union type */

type::~type() noexcept
{}

std::string
type::debug_string() const
{
	return "unn";
}

bool
type::operator==(const jive::type & other) const noexcept
{
	auto type = dynamic_cast<const jive::unn::type*>(&other);
	return type
	    && declaration() == type->declaration();
}

std::unique_ptr<jive::type>
type::copy() const
{
	return std::unique_ptr<jive::type>(new type(*this));
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
	    && result_ == op->result_
	    && argument_ == op->argument_;
}

std::string
choose_op::debug_string() const
{
	return detail::strfmt("CHOOSE(", option(), ")");
}

const jive::port &
choose_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return argument_;
}

const jive::port &
choose_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return result_;
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

		auto decl = static_cast<const jive::unn::type*>(
			&arg->node()->output(0)->type())->declaration();

		size_t nstates = arg->node()->ninputs()-1;
		std::vector<jive::output*> states;
		for (size_t n = 0; n < nstates; n++)
			states.push_back(arg->node()->input(n+1)->origin());

		if (dynamic_cast<const jive::addrtype*>(&address->type())) {
			return jive_load_by_address_create(address, decl->elements[option()],
				nstates, &states[0]);
		} else {
			size_t nbits = static_cast<const jive::bits::type*>(&address->type())->nbits();
			return jive_load_by_bitstring_create(address, nbits, decl->elements[option()],
				nstates, &states[0]);
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
	    && result_ == op->result_
	    && argument_ == op->argument_;
}

std::string
unify_op::debug_string() const
{
	return detail::strfmt("UNIFY(", option(), ")");
}

const jive::port &
unify_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return argument_;
}

const jive::port &
unify_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return result_;
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

}}

jive::output *
jive_unify_create(
	const jive::unn::declaration * decl,
	size_t option,
	jive::output * const argument)
{
	const jive::unn::type  unn_type(decl);
	jive::unn::unify_op op(unn_type, option);
	return jive::create_normalized(argument->region(), op, {argument})[0];
}

/* empty unify operation */

namespace jive {
namespace unn {

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

}}

jive::output *
jive_empty_unify_create(
	jive::region * region,
	const jive::unn::declaration * decl)
{
	jive::unn::empty_unify_op op(decl);
	return jive::create_normalized(region, op, {})[0];
}
