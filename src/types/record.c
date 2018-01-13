/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/address-transform.h>
#include <jive/arch/load.h>
#include <jive/types/bitstring/type.h>
#include <jive/types/record.h>

static constexpr jive_unop_reduction_path_t jive_select_reduction_load = 128;

namespace jive {
namespace rcd {

/* record type */

type::~type() noexcept
{}

std::string
type::debug_string() const
{
	return "rcd";
}

bool
type::operator==(const jive::type & other) const noexcept
{
	auto type = dynamic_cast<const jive::rcd::type*>(&other);
	return type != nullptr
	    && declaration() == type->declaration();
}

std::unique_ptr<jive::type>
type::copy() const
{
	return std::unique_ptr<jive::type>(new type(*this));
}

/* group operator */

group_op::~group_op() noexcept
{}

bool
group_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const group_op*>(&other);
	return op && op->result_ == result_ && op->arguments_ == arguments_;
}

size_t
group_op::narguments() const noexcept
{
	return arguments_.size();
}

const jive::port &
group_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return arguments_[index];
}

size_t
group_op::nresults() const noexcept
{
	return 1;
}

const jive::port &
group_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return result_;
}

std::string
group_op::debug_string() const
{
	return "GROUP";
}

std::unique_ptr<jive::operation>
group_op::copy() const
{
	return std::unique_ptr<jive::operation>(new group_op(*this));
}

}}

jive::output *
jive_group_create(std::shared_ptr<const jive::rcd::declaration> & decl,
	size_t narguments, jive::output * const * arguments)
{
	jive::rcd::group_op op(decl);
	jive::region * region = arguments[0]->region();
	return jive::create_normalized(
		region, op, std::vector<jive::output*>(arguments, arguments + narguments))[0];
}

jive::output *
jive_empty_group_create(jive::graph * graph,
	std::shared_ptr<const jive::rcd::declaration> & decl)
{
	jive::rcd::group_op op(decl);
	return jive::create_normalized(graph->root(), op, {})[0];
}

namespace jive {
namespace rcd {

/* select operator */

select_op::~select_op() noexcept
{}

bool
select_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const select_op*>(&other);
	return op
	    && op->index_ == index_
	    && op->result_ == result_
	    && op->argument_ == argument_;
}

std::string
select_op::debug_string() const
{
	return detail::strfmt("SELECT(", index(), ")");
}

const jive::port &
select_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return argument_;
}

const jive::port &
select_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return result_;
}

jive_unop_reduction_path_t
select_op::can_reduce_operand(const jive::output * arg) const noexcept
{
	if (arg->node() && dynamic_cast<const group_op *>(&arg->node()->operation()))
		return jive_unop_reduction_inverse;

	if (arg->node() && dynamic_cast<const load_op *>(&arg->node()->operation()))
		return jive_select_reduction_load;

	return jive_unop_reduction_none;
}

jive::output *
select_op::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	if (path == jive_unop_reduction_inverse)
		return arg->node()->input(index())->origin();

	if (path == jive_select_reduction_load) {
		auto address = arg->node()->input(0)->origin();

		size_t nbits = 0;
		if (auto bt = dynamic_cast<const jive::bits::type*>(&address->type())) {
			nbits = bt->nbits();
			address = bit2addr_op::create(address, bt->nbits(), address->type());
		}

		std::shared_ptr<const jive::rcd::declaration> decl;
		decl = static_cast<const jive::rcd::type*>(&arg->node()->output(0)->type())->declaration();

		size_t nstates = arg->node()->ninputs()-1;
		std::vector<jive::output*> states;
		for (size_t n = 0; n < nstates; n++)
			states.push_back(arg->node()->input(n+1)->origin());

		auto element_address = jive_memberof(address, decl, index());
		if (dynamic_cast<const jive::addrtype*>(&address->type())) {
			return jive_load_by_address_create(element_address, &decl->element(index()),
				nstates, &states[0]);
		} else {
			return jive_load_by_bitstring_create(element_address, nbits, &decl->element(index()),
				nstates, &states[0]);
		}
	}

	return nullptr;
}

std::unique_ptr<jive::operation>
select_op::copy() const
{
	return std::unique_ptr<jive::operation>(new select_op(*this));
}

}}
