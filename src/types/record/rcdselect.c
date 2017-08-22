/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/record.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/address.h>
#include <jive/arch/load.h>
#include <jive/types/bitstring/type.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/simple_node.h>

static constexpr jive_unop_reduction_path_t jive_select_reduction_load = 128;

namespace jive {
namespace rcd {

select_operation::~select_operation() noexcept
{
}

bool
select_operation::operator==(const operation & other) const noexcept
{
	const select_operation * op =
		dynamic_cast<const select_operation *>(&other);
	return op && type_ == op->type_ && element_ == op->element_;
}
std::string
select_operation::debug_string() const
{
	return detail::strfmt("SELECT(", element(), ")");
}

const jive::base::type &
select_operation::argument_type(size_t index) const noexcept
{
	return type_;
}

const jive::base::type &
select_operation::result_type(size_t index) const noexcept
{
	return type_.declaration()->element(element());
}

jive_unop_reduction_path_t
select_operation::can_reduce_operand(
	const jive::output * arg) const noexcept
{
	if (arg->node() && dynamic_cast<const group_op *>(&arg->node()->operation()))
		return jive_unop_reduction_inverse;

	if (arg->node() && dynamic_cast<const load_op *>(&arg->node()->operation()))
		return jive_select_reduction_load;

	return jive_unop_reduction_none;
}

jive::output *
select_operation::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	if (path == jive_unop_reduction_inverse)
		return arg->node()->input(element())->origin();

	if (path == jive_select_reduction_load) {
		auto address = arg->node()->input(0)->origin();

		size_t nbits = 0;
		if (dynamic_cast<const jive::bits::type*>(&address->type())) {
			nbits = static_cast<const jive::bits::type*>(&address->type())->nbits();
			address = jive_bitstring_to_address_create(address, nbits, &address->type());
		}
		
		std::shared_ptr<const jive::rcd::declaration> decl;
		decl = static_cast<const jive::rcd::type*>(&arg->node()->output(0)->type())->declaration();

		size_t nstates = arg->node()->ninputs()-1;
		jive::output * states[nstates];
		for (size_t n = 0; n < nstates; n++) {
			states[n] = arg->node()->input(n+1)->origin();
		}

		auto element_address = jive_memberof(address, decl, element());
		if (dynamic_cast<const jive::addr::type*>(&address->type())) {
			return jive_load_by_address_create(element_address, &decl->element(element()),
				nstates, states);
		} else {
			return jive_load_by_bitstring_create(element_address, nbits, &decl->element(element()),
				nstates, states);
		}
	}

	return nullptr;
}

std::unique_ptr<jive::operation>
select_operation::copy() const
{
	return std::unique_ptr<jive::operation>(new select_operation(*this));
}

}
}


jive::output *
jive_select_create(size_t member, jive::output * argument)
{
	const jive::rcd::type & rcd_type =
		dynamic_cast<const jive::rcd::type &>(argument->type());
	jive::rcd::select_operation op(rcd_type, member);
	return jive::create_normalized(argument->region(), op, {argument})[0];
}

