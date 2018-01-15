/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/address-transform.h>

#include <jive/arch/address.h>
#include <jive/arch/addresstype.h>
#include <jive/arch/call.h>
#include <jive/arch/load.h>
#include <jive/arch/memlayout.h>
#include <jive/arch/store.h>
#include <jive/rvsdg/label.h>
#include <jive/rvsdg/region.h>
#include <jive/rvsdg/substitution.h>
#include <jive/rvsdg/structural-node.h>
#include <jive/rvsdg/traverser.h>
#include <jive/types/bitstring/arithmetic.h>
#include <jive/types/bitstring/constant.h>
#include <jive/types/bitstring/type.h>
#include <jive/types/function/fctapply.h>
#include <jive/types/function/fctlambda.h>
#include <jive/types/function/fcttype.h>

static bool
type_contains_address(const jive::type * type)
{
	if (dynamic_cast<const jive::addrtype*>(type))
		return true;

	const jive::fct::type * fcttype = dynamic_cast<const jive::fct::type*>(type);
	if (fcttype != NULL) {
		size_t n;
		for (n = 0; n < fcttype->narguments(); n++)
			if (type_contains_address(&fcttype->argument_type(n)))
				return true;
		for (n = 0; n < fcttype->nresults(); n++)
			if (type_contains_address(&fcttype->result_type(n)))
				return true;
	}

	return false;
}

static std::unique_ptr<jive::type>
convert_address_to_bitstring_type(
	const jive::type & type,
	size_t nbits)
{
	if (dynamic_cast<const jive::addrtype*>(&type)) {
		return std::unique_ptr<jive::type>(new jive::bits::type(nbits));
	} else  if (auto fcttype = dynamic_cast<const jive::fct::type*>(&type)) {
		std::vector<std::unique_ptr<jive::type>> argument_types;
		for (size_t n = 0; n < fcttype->narguments(); n++)
			argument_types.push_back(convert_address_to_bitstring_type(fcttype->argument_type(n), nbits));

		std::vector<std::unique_ptr<jive::type>> return_types;
		for (size_t n = 0; n < fcttype->nresults(); n++)
			return_types.push_back(convert_address_to_bitstring_type(fcttype->result_type(n), nbits));

		return std::unique_ptr<jive::type>(new jive::fct::type(argument_types, return_types));
	}

	if (dynamic_cast<const jive::memtype*>(&type))
		return std::unique_ptr<jive::type>(new jive::memtype());

	if (dynamic_cast<const jive::bits::type*>(&type))
		return std::unique_ptr<jive::type>(type.copy());

	JIVE_ASSERT(0);
}

/* reductions */

static void
jive_load_node_address_transform(
	jive::node * node,
	const jive::load_op & op,
	size_t nbits)
{
	bool input_is_address = dynamic_cast<const jive::addrtype*>(&node->input(0)->type());
	bool output_is_address = dynamic_cast<const jive::addrtype*>(&node->output(0)->type());

	if (!input_is_address && !output_is_address)
		return;

	auto address = node->input(0)->origin();
	if (input_is_address)
		address = jive::addr2bit_op::create(address, nbits, address->type());

	JIVE_DEBUG_ASSERT(static_cast<const jive::bits::type*>(&address->type())->nbits() == nbits);

	jive::bits::type bits(nbits);
	auto datatype = &op.valuetype();
	if (output_is_address)
		datatype = &bits;

	std::vector<jive::output*> states;
	size_t nstates = node->ninputs() - 1;
	for (size_t i = 0; i < nstates; i++)
		states.push_back(node->input(i+1)->origin());

	auto load = jive::bitload_op::create(address, nbits, *datatype, states);
	
	if (output_is_address)
		load = jive::bit2addr_op::create(load, nbits, node->output(0)->type());
	
	node->output(0)->replace(load);
}

static void
jive_store_node_address_transform(
	jive::node * node,
	const jive::store_op & op,
	size_t nbits)
{
	bool input0_is_address = dynamic_cast<const jive::addrtype*>(&node->input(0)->type());
	bool input1_is_address = dynamic_cast<const jive::addrtype*>(&node->input(1)->type());

	if (!input0_is_address && !input1_is_address)
		return;

	auto address = node->input(0)->origin();
	if (input0_is_address)
		address = jive::addr2bit_op::create(address, nbits, address->type());

	JIVE_DEBUG_ASSERT(static_cast<const jive::bits::type*>(&address->type())->nbits() == nbits);

	jive::bits::type bits(nbits);
	auto datatype = &op.valuetype();
	auto value = node->input(1)->origin();
	if (input1_is_address){
		datatype = &bits;
		value = jive::addr2bit_op::create(node->input(1)->origin(), nbits, value->type());
	}

	std::vector<jive::output*> states;
	size_t nstates = node->ninputs() - 2;
	for (size_t n = 0; n < nstates; ++n)
		states.push_back(node->input(n+2)->origin());

	auto ostates = jive::bitstore_op::create(address, value, nbits, *datatype, states);

	for (size_t n = 0; n < nstates; ++n)
		node->output(n)->replace(ostates[n]);
}

static void
jive_label_to_address_node_address_transform(
	jive::node * node,
	const jive::address::label_to_address_op & op,
	size_t nbits)
{
	auto label_o = jive_label_to_bitstring_create(node->graph()->root(), op.label(), nbits);
	auto addr_o = jive::bit2addr_op::create(label_o, nbits, node->output(0)->type());
	node->output(0)->replace(addr_o);
}

static void
jive_call_node_address_transform(
	jive::node * node,
	const jive::call_operation & op,
	size_t nbits)
{
	bool transform = false;
	std::vector<jive::output*> operands(node->ninputs());
	for (size_t i = 0; i < node->ninputs(); i++){
		if(dynamic_cast<const jive::addrtype*>(&node->input(i)->type())){
			operands[i] = jive::addr2bit_op::create(node->input(i)->origin(), nbits,
				node->input(i)->origin()->type());
			transform = true;
		} else {
			operands[i] = node->input(i)->origin();
		}
	}

	jive::bits::type address_type(nbits);
	std::vector<const jive::type*> return_types;
	for (size_t i = 0; i < node->noutputs(); i++){
		if (dynamic_cast<const jive::addrtype*>(&node->output(i)->type())){
			return_types.push_back(&address_type);
			transform = true;
		} else {
			return_types.push_back(&node->output(i)->type());
		}
	}

	if (!transform)
		return;

	auto call = jive_call_by_bitstring_node_create(node->region(), operands[0], nbits,
		op.calling_convention(), node->ninputs() - 1, &operands[0] + 1, node->noutputs(),
		&return_types[0]);

	for (size_t i = 0; i < node->noutputs(); i++){
		auto output = call->output(i);
		if(dynamic_cast<const jive::addrtype*>(&node->output(i)->type()))
			output = jive::bit2addr_op::create(call->output(i), nbits, node->output(i)->type());
		node->output(i)->replace(output);
	}
}

static void
jive_memberof_node_address_transform(
	jive::node * node,
	const jive::address::memberof_op & op,
	jive::memlayout_mapper * mapper)
{
	size_t index = op.index();
	auto dcl = op.record_decl();

	JIVE_DEBUG_ASSERT(index < dcl->nelements());
	size_t elem_offset = mapper->map_record(dcl).element(index).offset();
	size_t nbits = mapper->map_address().size() * 8;

	auto offset = create_bitconstant(node->region(), nbits, elem_offset);
	auto address = jive::addr2bit_op::create(node->input(0)->origin(), nbits,
		node->input(0)->origin()->type());
	auto sum = jive::bits::create_add(nbits, address, offset);
	auto off_address = jive::bit2addr_op::create(sum, nbits, node->output(0)->type());

	node->output(0)->replace(off_address);
}

static void
jive_containerof_node_address_transform(
	jive::node * node,
	const jive::address::containerof_op & op,
	jive::memlayout_mapper * mapper)
{
	size_t index = op.index();
	auto dcl = op.record_decl();

	JIVE_DEBUG_ASSERT(index < dcl->nelements());
	size_t elem_offset = mapper->map_record(dcl).element(index).offset();
	size_t nbits = mapper->map_address().size() * 8;

	auto offset = create_bitconstant(node->region(), nbits, elem_offset);
	auto address = jive::addr2bit_op::create(node->input(0)->origin(), nbits,
		node->input(0)->origin()->type());
	auto sum = jive::bits::create_sub(nbits, address, offset);
	auto off_address = jive::bit2addr_op::create(sum, nbits, node->output(0)->type());

	node->output(0)->replace(off_address);
}

static void
jive_arraysubscript_node_address_transform(
	jive::node * node,
	const jive::address::arraysubscript_op & op,
	jive::memlayout_mapper * mapper)
{
	size_t elem_type_size = mapper->map_value_type(op.element_type()).size();
	size_t nbits = mapper->map_address().size() * 8;

	auto index = node->input(1)->origin();
	auto address = jive::addr2bit_op::create(node->input(0)->origin(), nbits,
		node->input(0)->origin()->type());
	auto elem_size = create_bitconstant(node->region(), nbits, elem_type_size);
	auto offset = jive::bits::create_mul(nbits, elem_size, index);
	auto sum = jive::bits::create_add(nbits, address, offset);
	auto off_address = jive::bit2addr_op::create(sum, nbits, node->output(0)->type());
	
	node->output(0)->replace(off_address);
}

static void
jive_arrayindex_node_address_transform(
	jive::node * node,
	const jive::address::arrayindex_op & op,
	jive::memlayout_mapper * mapper)
{
	size_t elem_type_size = mapper->map_value_type(op.element_type()).size();
	size_t nbits = mapper->map_address().size() * 8;

	auto address1 = jive::addr2bit_op::create(node->input(0)->origin(), nbits,
		node->input(0)->origin()->type());
	auto address2 = jive::addr2bit_op::create(node->input(1)->origin(), nbits,
		node->input(1)->origin()->type());
	auto elem_size = create_bitconstant(node->region(), nbits, elem_type_size);
	auto diff = jive::bits::create_sub(nbits, address1, address2);
	auto div = jive::bits::create_sdiv(nbits, diff, elem_size);

	node->output(0)->replace(div);
}

static void
jive_apply_node_address_transform(const jive::node * node, size_t nbits)
{
	auto fct = dynamic_cast<jive::simple_input*>(node->input(0));

	auto & fcttype = fct->type();
	if (!type_contains_address(&fcttype))
		return;

	std::vector<jive::output*> arguments;
	for (size_t n = 1; n < node->ninputs(); n++) {
		auto argument = node->input(n);
		arguments.push_back(jive::addr2bit_op::create(argument->origin(), nbits, argument->type()));
	}
	auto function = jive::addr2bit_op::create(fct->origin(), nbits, fcttype);

	auto results = jive::fct::create_apply(function, arguments);

	for (size_t n = 0; n < results.size(); n++) {
		auto original = node->output(n);
		auto substitute = jive::bit2addr_op::create(results[n], nbits, original->type());
		original->replace(substitute);
	}
}

void
jive_graph_address_transform(jive::graph * graph, jive::memlayout_mapper * mapper)
{
	for (jive::node * node : jive::topdown_traverser(graph->root())) {
		jive_node_address_transform(node, mapper);
	}
}

void
jive_node_address_transform(jive::node * node, jive::memlayout_mapper * mapper)
{
	size_t nbits = mapper->map_address().size() * 8;
	if (auto op = dynamic_cast<const jive::address::memberof_op *>(&node->operation())) {
		jive_memberof_node_address_transform(node, *op, mapper);
	} else if (auto op = dynamic_cast<const jive::address::containerof_op *>(&node->operation())) {
		jive_containerof_node_address_transform(node, *op, mapper);
	} else if (auto op = dynamic_cast<const jive::address::arrayindex_op *>(&node->operation())) {
		jive_arrayindex_node_address_transform(node, *op, mapper);
	} else if (auto op = dynamic_cast<const jive::address::arraysubscript_op *>(&node->operation())) {
		jive_arraysubscript_node_address_transform(node, *op, mapper);
	} else if (auto op = dynamic_cast<const jive::address::label_to_address_op *>(
		&node->operation())) {
		jive_label_to_address_node_address_transform(node, *op, nbits);
	} else if (auto op = dynamic_cast<const jive::load_op *>(&node->operation())) {
		jive_load_node_address_transform(node, *op, nbits);
	} else if (auto op = dynamic_cast<const jive::store_op *>(&node->operation())) {
		jive_store_node_address_transform(node, *op, nbits);
	} else if (auto op = dynamic_cast<const jive::call_operation *>(&node->operation())) {
		jive_call_node_address_transform(node, *op, nbits);
	} else if (dynamic_cast<const jive::fct::apply_op *>(&node->operation())) {
		jive_apply_node_address_transform(node, nbits);
	}
}

namespace jive {

addr2bit_op::~addr2bit_op() noexcept
{}

addr2bit_op::addr2bit_op(
	size_t nbits,
	const jive::type & type)
: nbits_(nbits)
, result_(*convert_address_to_bitstring_type(type, nbits))
, argument_(type)
{}

jive_unop_reduction_path_t
addr2bit_op::can_reduce_operand(const jive::output * arg) const noexcept
{
	if (!type_contains_address(&arg->type()))
		return jive_unop_reduction_idempotent;

	if (!arg->node())
		return jive_unop_reduction_none;

	if (auto op = dynamic_cast<const bit2addr_op*>(&arg->node()->operation())) {
		if (op->nbits() != nbits())
			return jive_unop_reduction_none;

		if (op->original_type() != argument_.type())
			return jive_unop_reduction_none;

		return jive_unop_reduction_inverse;
	}

	return jive_unop_reduction_none;
}

jive::output *
addr2bit_op::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	if (path == jive_unop_reduction_inverse)
		return arg->node()->input(0)->origin();

	if (path == jive_unop_reduction_idempotent)
		return arg;

	return nullptr;
}

const jive::port &
addr2bit_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return argument_;
}

const jive::port &
addr2bit_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return result_;
}

bool
addr2bit_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const addr2bit_op*>(&other);
	return op
	    && nbits() == op->nbits()
	    && result_ == op->result_
	    && argument_ == op->argument_;
}
std::string
addr2bit_op::debug_string() const
{
	return "ADDR2BIT";
}

std::unique_ptr<jive::operation>
addr2bit_op::copy() const
{
	return std::unique_ptr<jive::operation>(new addr2bit_op(*this));
}

/* bit2addr operator */

bit2addr_op::~bit2addr_op() noexcept
{}

bit2addr_op::bit2addr_op(
	size_t nbits,
	const jive::type & type)
: nbits_(nbits)
, result_(type)
, argument_(*convert_address_to_bitstring_type(type, nbits))
{}

jive_unop_reduction_path_t
bit2addr_op::can_reduce_operand(const jive::output * arg) const noexcept
{
	if (!type_contains_address(&original_type()))
		return jive_unop_reduction_idempotent;

	if (!arg->node())
		return jive_unop_reduction_none;

	if (auto op = dynamic_cast<const addr2bit_op*>(&arg->node()->operation())) {
		if (op->nbits() != nbits())
			return jive_unop_reduction_none;

		if (op->original_type() != result_.type())
			return jive_unop_reduction_none;

		return jive_unop_reduction_inverse;
	}


	return jive_unop_reduction_none;
}

jive::output *
bit2addr_op::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	if (path == jive_unop_reduction_inverse)
		return arg->node()->input(0)->origin();

	if (path == jive_unop_reduction_idempotent)
		return arg;

	return nullptr;
}

const jive::port &
bit2addr_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return argument_;
}

const jive::port &
bit2addr_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return result_;
}

bool
bit2addr_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const bit2addr_op*>(&other);
	return op
	    && nbits() == op->nbits()
	    && result_ == op->result_
	    && argument_ == op->argument_;
}
std::string
bit2addr_op::debug_string() const
{
	return "BIT2ADDR";
}

std::unique_ptr<jive::operation>
bit2addr_op::copy() const
{
	return std::unique_ptr<jive::operation>(new bit2addr_op(*this));
}

}
