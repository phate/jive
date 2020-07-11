/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/address.hpp>

#include <jive/arch/addresstype.hpp>
#include <jive/rvsdg/graph.hpp>
#include <jive/rvsdg/label.hpp>
#include <jive/rvsdg/region.hpp>
#include <jive/types/bitstring/arithmetic.hpp>
#include <jive/types/bitstring/constant.hpp>
#include <jive/types/bitstring/type.hpp>

/* memberof */

namespace jive {

memberof_op::~memberof_op() noexcept
{
}

bool
memberof_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const memberof_op *>(&other);
	return op
	    && op->record_decl() == record_decl()
	    && op->index() == index();
}

std::string
memberof_op::debug_string() const
{
	return detail::strfmt("MEMBEROF", record_decl(), index());
}

jive_unop_reduction_path_t
memberof_op::can_reduce_operand(
	const jive::output * arg) const noexcept
{
	auto tmp = dynamic_cast<const jive::simple_output*>(arg);
	if (!tmp)
		return jive_unop_reduction_none;

	auto op = dynamic_cast<const containerof_op *>(&tmp->node()->operation());
	if (!op)
		return jive_unop_reduction_none;

	if (op->record_decl() == record_decl() && op->index() == index()) {
		return jive_unop_reduction_inverse;
	}

	return jive_unop_reduction_none;
}

jive::output *
memberof_op::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	if (path == jive_unop_reduction_inverse)
		return node_output::node(arg)->input(0)->origin();
	
	return nullptr;
}

std::unique_ptr<jive::operation>
memberof_op::copy() const
{
	return std::unique_ptr<jive::operation>(new memberof_op(*this));
}

/* containerof */

containerof_op::~containerof_op() noexcept
{
}

bool
containerof_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const containerof_op *>(&other);
	return op
	    && op->record_decl() == record_decl()
	    && op->index() == index();
}

std::string
containerof_op::debug_string() const
{
	return detail::strfmt("CONTAINEROF", record_decl(), index());
}

jive_unop_reduction_path_t
containerof_op::can_reduce_operand(
	const jive::output * arg) const noexcept
{
	auto tmp = dynamic_cast<const jive::simple_output*>(arg);
	if (!tmp)
		return jive_unop_reduction_none;

	auto op = dynamic_cast<const memberof_op *>(&tmp->node()->operation());
	if (!op)
		return jive_unop_reduction_none;

	if (op->record_decl() == record_decl() && op->index() == index())
		return jive_unop_reduction_inverse;

	return jive_unop_reduction_none;
}

jive::output *
containerof_op::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	if (path == jive_unop_reduction_inverse)
		return node_output::node(arg)->input(0)->origin();

	return nullptr;
}

std::unique_ptr<jive::operation>
containerof_op::copy() const
{
	return std::unique_ptr<jive::operation>(new containerof_op(*this));
}

/* arraysubscript */

arraysubscript_op::~arraysubscript_op() noexcept
{}

bool
arraysubscript_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const arraysubscript_op*>(&other);
	return op
	    && op->element_type() == element_type()
	    && op->argument(1) == argument(1);
}

std::string
arraysubscript_op::debug_string() const
{
	return "ARRAYSUBSCRIPT";
}

std::unique_ptr<jive::operation>
arraysubscript_op::copy() const
{
	return std::unique_ptr<jive::operation>(new arraysubscript_op(*this));
}

/* arrayindex */

arrayindex_op::~arrayindex_op() noexcept
{
}

bool
arrayindex_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const arrayindex_op*>(&other);
	return op
	    && op->element_type() == element_type()
	    && op->result(0) == result(0);
}

std::string
arrayindex_op::debug_string() const
{
	return "ARRAYINDEX";
}

std::unique_ptr<jive::operation>
arrayindex_op::copy() const
{
	return std::unique_ptr<jive::operation>(new arrayindex_op(*this));
}

/* lbl2addr operation */

lbl2addr_op::~lbl2addr_op() noexcept
{}

bool
lbl2addr_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const lbl2addr_op*>(&other);
	return op && op->label() == label();
}

std::string
lbl2addr_op::debug_string() const
{
	return detail::strfmt("LBL2ADDR: ", label());
}

std::unique_ptr<jive::operation>
lbl2addr_op::copy() const
{
	return std::unique_ptr<jive::operation>(new lbl2addr_op(*this));
}

/* lbl2bit operation */

lbl2bit_op::~lbl2bit_op() noexcept
{}

bool
lbl2bit_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const lbl2bit_op*>(&other);
	return op
	    && op->label() == label()
	    && op->nbits() == nbits();
}

std::string
lbl2bit_op::debug_string() const
{
	return detail::strfmt("LBL2BIT: ", label());
}

std::unique_ptr<jive::operation>
lbl2bit_op::copy() const
{
	return std::unique_ptr<jive::operation>(new lbl2bit_op(*this));
}

/* constant */

jive::output *
constant(jive::graph * graph, const value_repr & vr)
{
	addrconstant_op op(vr);
	return jive::simple_node::create_normalized(graph->root(), op, {})[0];
}

}
