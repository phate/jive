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

namespace jive {

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
		return std::unique_ptr<jive::type>(new jive::bittype(nbits));
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

	if (dynamic_cast<const jive::bittype*>(&type))
		return std::unique_ptr<jive::type>(type.copy());

	JIVE_ASSERT(0);
}

/* reductions */

static void
transform_load(jive::node * node, memlayout_mapper & mapper)
{
	JIVE_DEBUG_ASSERT(is_load_node(node));
	auto op = static_cast<const jive::load_op*>(&node->operation());
	size_t nbits = mapper.map_address().size()*8;

	bool input_is_address = dynamic_cast<const addrtype*>(&node->input(0)->type());
	bool output_is_address = dynamic_cast<const addrtype*>(&node->output(0)->type());

	if (!input_is_address && !output_is_address)
		return;

	auto address = node->input(0)->origin();
	if (input_is_address)
		address = jive::addr2bit_op::create(address, nbits, address->type());

	JIVE_DEBUG_ASSERT(static_cast<const bittype*>(&address->type())->nbits() == nbits);

	bittype bits(nbits);
	auto datatype = &op->valuetype();
	if (output_is_address)
		datatype = &bits;

	std::vector<jive::output*> states;
	size_t nstates = node->ninputs() - 1;
	for (size_t i = 0; i < nstates; i++)
		states.push_back(node->input(i+1)->origin());

	auto load = bitload_op::create(address, nbits, *datatype, states);
	
	if (output_is_address)
		load = bit2addr_op::create(load, nbits, node->output(0)->type());
	
	node->output(0)->divert_users(load);
}

static void
transform_store(jive::node * node, memlayout_mapper & mapper)
{
	JIVE_DEBUG_ASSERT(is_store_node(node));
	auto op = static_cast<const jive::store_op*>(&node->operation());
	size_t nbits = mapper.map_address().size()*8;

	bool input0_is_address = dynamic_cast<const addrtype*>(&node->input(0)->type());
	bool input1_is_address = dynamic_cast<const addrtype*>(&node->input(1)->type());

	if (!input0_is_address && !input1_is_address)
		return;

	auto address = node->input(0)->origin();
	if (input0_is_address)
		address = addr2bit_op::create(address, nbits, address->type());

	JIVE_DEBUG_ASSERT(static_cast<const bittype*>(&address->type())->nbits() == nbits);

	jive::bittype bits(nbits);
	auto datatype = &op->valuetype();
	auto value = node->input(1)->origin();
	if (input1_is_address){
		datatype = &bits;
		value = addr2bit_op::create(node->input(1)->origin(), nbits, value->type());
	}

	std::vector<jive::output*> states;
	size_t nstates = node->ninputs() - 2;
	for (size_t n = 0; n < nstates; ++n)
		states.push_back(node->input(n+2)->origin());

	auto ostates = bitstore_op::create(address, value, nbits, *datatype, states);

	divert_users(node, ostates);
}

static void
transform_lbl2addr(jive::node * node, memlayout_mapper & mapper)
{
	JIVE_DEBUG_ASSERT(is_lbl2addr_node(node));
	auto op = static_cast<const lbl2addr_op*>(&node->operation());
	size_t nbits = mapper.map_address().size()*8;

	auto label_o = lbl2bit_op::create(node->graph()->root(), nbits, op->label());
	auto addr_o = bit2addr_op::create(label_o, nbits, node->output(0)->type());
	node->output(0)->divert_users(addr_o);
}

static void
transform_call(jive::node * node, memlayout_mapper & mapper)
{
	JIVE_DEBUG_ASSERT(is_call_node(node));
	auto op = static_cast<const call_op*>(&node->operation());
	size_t nbits = mapper.map_address().size()*8;

	bool transform = false;
	std::vector<jive::output*> operands;
	for (size_t i = 1; i < node->ninputs(); i++){
		auto origin = node->input(i)->origin();
		if(dynamic_cast<const addrtype*>(&origin->type())) {
			operands.push_back(addr2bit_op::create(origin, nbits, origin->type()));
			transform = true;
		} else
			operands.push_back(node->input(i)->origin());
	}

	jive::bittype address_type(nbits);
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

	auto address = node->input(0)->origin();
	address = addr2bit_op::create(address, nbits, address->type());
	auto call = bitcall_op::create(address, nbits, operands, return_types, op->calling_convention());

	for (size_t i = 0; i < node->noutputs(); i++){
		auto output = call[i];
		if(dynamic_cast<const jive::addrtype*>(&node->output(i)->type()))
			output = bit2addr_op::create(output, nbits, node->output(i)->type());
		node->output(i)->divert_users(output);
	}
}

static void
transform_memberof(jive::node * node, memlayout_mapper & mapper)
{
	JIVE_DEBUG_ASSERT(dynamic_cast<const memberof_op*>(&node->operation()));
	auto op = static_cast<const memberof_op*>(&node->operation());
	size_t nbits = mapper.map_address().size()*8;

	size_t elem_offset = mapper.map_record(op->record_decl()).element(op->index()).offset();

	auto offset = create_bitconstant(node->region(), nbits, elem_offset);
	auto address = addr2bit_op::create(node->input(0)->origin(), nbits,
		node->input(0)->origin()->type());
	auto sum = bitadd_op::create(nbits, address, offset);
	auto off_address = bit2addr_op::create(sum, nbits, node->output(0)->type());

	node->output(0)->divert_users(off_address);
}

static void
transform_containerof(jive::node * node, memlayout_mapper & mapper)
{
	JIVE_DEBUG_ASSERT(dynamic_cast<const containerof_op*>(&node->operation()));
	auto op = static_cast<const containerof_op*>(&node->operation());
	size_t nbits = mapper.map_address().size()*8;

	size_t elem_offset = mapper.map_record(op->record_decl()).element(op->index()).offset();

	auto offset = create_bitconstant(node->region(), nbits, elem_offset);
	auto address = addr2bit_op::create(node->input(0)->origin(), nbits,
		node->input(0)->origin()->type());
	auto sum = bitsub_op::create(nbits, address, offset);
	auto off_address = bit2addr_op::create(sum, nbits, node->output(0)->type());

	node->output(0)->divert_users(off_address);
}

static void
transform_arraysubscript(jive::node * node, memlayout_mapper & mapper)
{
	JIVE_DEBUG_ASSERT(dynamic_cast<const arraysubscript_op*>(&node->operation()));
	auto op = static_cast<const arraysubscript_op*>(&node->operation());
	size_t nbits = mapper.map_address().size()*8;

	size_t elem_type_size = mapper.map_value_type(op->element_type()).size();

	auto index = node->input(1)->origin();
	auto address = addr2bit_op::create(node->input(0)->origin(), nbits,
		node->input(0)->origin()->type());
	auto elem_size = create_bitconstant(node->region(), nbits, elem_type_size);
	auto offset = bitmul_op::create(nbits, elem_size, index);
	auto sum = bitadd_op::create(nbits, address, offset);
	auto off_address = bit2addr_op::create(sum, nbits, node->output(0)->type());
	
	node->output(0)->divert_users(off_address);
}

static void
transform_arrayindex(jive::node * node, memlayout_mapper & mapper)
{
	JIVE_DEBUG_ASSERT(dynamic_cast<const arrayindex_op*>(&node->operation()));
	auto op = static_cast<const arrayindex_op*>(&node->operation());
	size_t nbits = mapper.map_address().size()*8;

	size_t elem_type_size = mapper.map_value_type(op->element_type()).size();
	auto address1 = addr2bit_op::create(node->input(0)->origin(), nbits,
		node->input(0)->origin()->type());
	auto address2 = addr2bit_op::create(node->input(1)->origin(), nbits,
		node->input(1)->origin()->type());
	auto elem_size = create_bitconstant(node->region(), nbits, elem_type_size);
	auto diff = bitsub_op::create(nbits, address1, address2);
	auto div = bitsdiv_op::create(nbits, diff, elem_size);

	node->output(0)->divert_users(div);
}

static void
transform_apply(jive::node * node, memlayout_mapper & mapper)
{
	JIVE_DEBUG_ASSERT(dynamic_cast<const jive::fct::apply_op*>(&node->operation()));
	size_t nbits = mapper.map_address().size()*8;
	auto fct = node->input(0);

	auto & fcttype = fct->type();
	if (!type_contains_address(&fcttype))
		return;

	std::vector<jive::output*> arguments;
	for (size_t n = 1; n < node->ninputs(); n++) {
		auto argument = node->input(n);
		arguments.push_back(addr2bit_op::create(argument->origin(), nbits, argument->type()));
	}
	auto function = addr2bit_op::create(fct->origin(), nbits, fcttype);

	auto results = fct::create_apply(function, arguments);

	for (size_t n = 0; n < results.size(); n++) {
		auto original = node->output(n);
		auto substitute = bit2addr_op::create(results[n], nbits, original->type());
		original->divert_users(substitute);
	}
}

void
transform_address(jive::node * node, memlayout_mapper & mapper)
{
	static std::unordered_map<std::type_index, void(*)(jive::node*, memlayout_mapper&)> map({
	  {std::type_index(typeid(memberof_op)), transform_memberof}
	, {std::type_index(typeid(containerof_op)), transform_containerof}
	, {std::type_index(typeid(arrayindex_op)), transform_arrayindex}
	, {std::type_index(typeid(arraysubscript_op)), transform_arraysubscript}
	, {std::type_index(typeid(lbl2addr_op)), transform_lbl2addr}
	, {std::type_index(typeid(bitload_op)), transform_load}
	, {std::type_index(typeid(addrload_op)), transform_load}
	, {std::type_index(typeid(bitstore_op)), transform_store}
	, {std::type_index(typeid(addrstore_op)), transform_store}
	, {std::type_index(typeid(bitcall_op)), transform_call}
	, {std::type_index(typeid(addrcall_op)), transform_call}
	, {std::type_index(typeid(fct::apply_op)), transform_apply}
	});

	if (map.find(typeid(node->operation())) != map.end())
		map[typeid(node->operation())](node, mapper);
}

void
transform_address(jive::graph * graph, memlayout_mapper & mapper)
{
	for (auto node : jive::topdown_traverser(graph->root()))
		transform_address(node, mapper);
}

/* addr2bit operator */

addr2bit_op::~addr2bit_op() noexcept
{}

addr2bit_op::addr2bit_op(
	size_t nbits,
	const jive::type & type)
: unary_op(type, *convert_address_to_bitstring_type(type, nbits))
, nbits_(nbits)
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

		if (op->original_type() != argument(0).type())
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

bool
addr2bit_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const addr2bit_op*>(&other);
	return op
	    && op->nbits() == nbits()
	    && op->original_type() == original_type();
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
: unary_op(*convert_address_to_bitstring_type(type, nbits), type)
, nbits_(nbits)
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

		if (op->original_type() != result(0).type())
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

bool
bit2addr_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const bit2addr_op*>(&other);
	return op
	    && op->nbits() == nbits()
	    && op->original_type() == original_type();
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
