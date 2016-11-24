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
#include <jive/arch/memorytype.h>
#include <jive/arch/store.h>
#include <jive/types/bitstring/arithmetic.h>
#include <jive/types/bitstring/constant.h>
#include <jive/types/bitstring/type.h>
#include <jive/types/function/fctapply.h>
#include <jive/types/function/fctlambda.h>
#include <jive/types/function/fcttype.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/substitution.h>
#include <jive/vsdg/traverser.h>

static bool
type_contains_address(const jive::base::type * type)
{
	if (dynamic_cast<const jive::addr::type*>(type))
		return true;

	const jive::fct::type * fcttype = dynamic_cast<const jive::fct::type*>(type);
	if (fcttype != NULL) {
		size_t n;
		for (n = 0; n < fcttype->narguments(); n++)
			if (type_contains_address(fcttype->argument_type(n)))
				return true;
		for (n = 0; n < fcttype->nreturns(); n++)
			if (type_contains_address(fcttype->return_type(n)))
				return true;
	}

	return false;
}

static std::unique_ptr<jive::base::type>
convert_address_to_bitstring_type(
	const jive::base::type & type,
	size_t nbits)
{
	if (dynamic_cast<const jive::addr::type*>(&type)) {
		return std::unique_ptr<jive::base::type>(new jive::bits::type(nbits));
	} else  if (auto fcttype = dynamic_cast<const jive::fct::type*>(&type)) {
		std::vector<std::unique_ptr<jive::base::type>> argument_types;
		for (size_t n = 0; n < fcttype->narguments(); n++)
			argument_types.push_back(convert_address_to_bitstring_type(*fcttype->argument_type(n), nbits));

		std::vector<std::unique_ptr<jive::base::type>> return_types;
		for (size_t n = 0; n < fcttype->nreturns(); n++)
			return_types.push_back(convert_address_to_bitstring_type(*fcttype->return_type(n), nbits));

		return std::unique_ptr<jive::base::type>(new jive::fct::type(argument_types, return_types));
	}

	if (dynamic_cast<const jive::mem::type*>(&type))
		return std::unique_ptr<jive::base::type>(new jive::mem::type());

	if (dynamic_cast<const jive::bits::type*>(&type))
		return std::unique_ptr<jive::base::type>(type.copy());

	JIVE_DEBUG_ASSERT(0);
}

jive::oport *
jive_address_to_bitstring_create(jive::oport * address, size_t nbits,
	const jive::base::type * original_type)
{
	jive::address_to_bitstring_operation op(
		nbits, std::unique_ptr<jive::base::type>(original_type->copy()));
	return jive_node_create_normalized(address->region(), op, {address})[0];
}

jive::oport *
jive_bitstring_to_address_create(jive::oport * bitstring, size_t nbits,
	const jive::base::type * original_type)
{
	jive::bitstring_to_address_operation op(
		nbits, std::unique_ptr<jive::base::type>(original_type->copy()));
	return jive_node_create_normalized(bitstring->region(), op, {bitstring})[0];
}

/* reductions */

static void
jive_load_node_address_transform(
	jive::node * node,
	const jive::load_op & op,
	size_t nbits)
{
	bool input_is_address = dynamic_cast<const jive::addr::type*>(&node->input(0)->type());
	bool output_is_address = dynamic_cast<const jive::addr::type*>(&node->output(0)->type());

	if (!input_is_address && !output_is_address)
		return;

	jive::oport * address = node->input(0)->origin();
	if (input_is_address)
		address = jive_address_to_bitstring_create(address, nbits, &address->type());

	JIVE_DEBUG_ASSERT(static_cast<const jive::bits::type*>(&address->type())->nbits() == nbits);

	jive::bits::type bits(nbits);
	const jive::value::type * datatype = &op.data_type();
	if (output_is_address)
		datatype = &bits;

	size_t i;
	size_t nstates = node->ninputs() - 1;
	jive::oport * states[nstates];
	for (i = 0; i < nstates; i++)
		states[i] = node->input(i+1)->origin();

	auto load = jive_load_by_bitstring_create(address, nbits, datatype, nstates, states);
	
	if (output_is_address)
		load = jive_bitstring_to_address_create(load, nbits, &node->output(0)->type());
	
	node->output(0)->replace(load);
}

static void
jive_store_node_address_transform(
	jive::node * node,
	const jive::store_op & op,
	size_t nbits)
{
	bool input0_is_address = dynamic_cast<const jive::addr::type*>(&node->input(0)->type());
	bool input1_is_address = dynamic_cast<const jive::addr::type*>(&node->input(1)->type());

	if (!input0_is_address && !input1_is_address)
		return;

	jive::oport * address = node->input(0)->origin();
	if (input0_is_address)
		address = jive_address_to_bitstring_create(address, nbits, &address->type());

	JIVE_DEBUG_ASSERT(static_cast<const jive::bits::type*>(&address->type())->nbits() == nbits);

	jive::bits::type bits(nbits);
	const jive::value::type * datatype = &op.data_type();
	jive::oport * value = node->input(1)->origin();
	if (input1_is_address){
		datatype = &bits;
		value = jive_address_to_bitstring_create(node->input(1)->origin(), nbits, &value->type());
	}

	size_t nstates = node->ninputs() - 2;
	jive::oport * states[nstates];
	for (size_t n = 0; n < nstates; ++n){
		states[n] = node->input(n+2)->origin();
	}

	auto ostates = jive_store_by_bitstring_create(address, nbits, datatype, value, nstates, states);

	for (size_t n = 0; n < nstates; ++n)
		node->output(n)->replace(ostates[n]);
}

static void
jive_label_to_address_node_address_transform(
	jive::node * node,
	const jive::address::label_to_address_op & op,
	size_t nbits)
{
	const jive_label * label = op.label();

	auto label_o = jive_label_to_bitstring_create(node->graph()->root(), label, nbits);
	auto addr_o = jive_bitstring_to_address_create(label_o, nbits, &node->output(0)->type());
	node->output(0)->replace(addr_o);
}

static void
jive_call_node_address_transform(
	jive::node * node,
	const jive::call_operation & op,
	size_t nbits)
{
	bool transform = false;
	jive::oport * operands[node->ninputs()];
	for (size_t i = 0; i < node->ninputs(); i++){
		if(dynamic_cast<const jive::addr::type*>(&node->input(i)->type())){
			operands[i] = jive_address_to_bitstring_create(node->input(i)->origin(), nbits,
				&node->input(i)->origin()->type());
			transform = true;
		} else {
			operands[i] = node->input(i)->origin();
		}
	}

	const jive::base::type * return_types[node->noutputs()];
	jive::bits::type address_type(nbits);
	for (size_t i = 0; i < node->noutputs(); i++){
		if (dynamic_cast<const jive::addr::type*>(&node->output(i)->type())){
			return_types[i] = &address_type;
			transform = true;
		} else {
			return_types[i] = &node->output(i)->type();
		}
	}

	if (!transform)
		return;

	jive::node * call = jive_call_by_bitstring_node_create(node->region(), operands[0], nbits,
		op.calling_convention(), node->ninputs() - 1, operands + 1, node->noutputs(),
		return_types);

	for (size_t i = 0; i < node->noutputs(); i++){
		jive::oport * output = call->output(i);
		if(dynamic_cast<const jive::addr::type*>(&node->output(i)->type()))
			output = jive_bitstring_to_address_create(call->output(i), nbits, &node->output(i)->type());
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
	std::shared_ptr<const jive::rcd::declaration> decl = op.record_decl();

	JIVE_DEBUG_ASSERT(index < decl->nelements());
	size_t elem_offset = mapper->map_record(decl).element(index).offset();
	size_t nbits = mapper->map_address().size() * 8;

	auto offset = jive_bitconstant_unsigned(node->region(), nbits, elem_offset);
	auto address = jive_address_to_bitstring_create(node->input(0)->origin(), nbits,
		&node->input(0)->origin()->type());
	auto sum = jive_bitsum({address, offset});
	auto off_address = jive_bitstring_to_address_create(sum, nbits, &node->output(0)->type());

	node->output(0)->replace(off_address);
}

static void
jive_containerof_node_address_transform(
	jive::node * node,
	const jive::address::containerof_op & op,
	jive::memlayout_mapper * mapper)
{
	size_t index = op.index();
	std::shared_ptr<const jive::rcd::declaration> decl = op.record_decl();

	JIVE_DEBUG_ASSERT(index < decl->nelements());
	size_t elem_offset = mapper->map_record(decl).element(index).offset();
	size_t nbits = mapper->map_address().size() * 8;

	auto offset = jive_bitconstant_unsigned(node->region(), nbits, elem_offset);
	auto address = jive_address_to_bitstring_create(node->input(0)->origin(), nbits,
		&node->input(0)->origin()->type());
	auto sum = jive_bitdifference(address, offset);
	auto off_address = jive_bitstring_to_address_create(sum, nbits, &node->output(0)->type());

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
	auto address = jive_address_to_bitstring_create(node->input(0)->origin(), nbits,
		&node->input(0)->origin()->type());
	auto elem_size = jive_bitconstant_unsigned(node->region(), nbits, elem_type_size);
	auto offset = jive_bitmultiply({elem_size, index});
	auto sum = jive_bitsum({address, offset});
	auto off_address = jive_bitstring_to_address_create(sum, nbits, &node->output(0)->type());
	
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

	auto address1 = jive_address_to_bitstring_create(node->input(0)->origin(), nbits,
		&node->input(0)->origin()->type());
	auto address2 = jive_address_to_bitstring_create(node->input(1)->origin(), nbits,
		&node->input(1)->origin()->type());
	auto elem_size = jive_bitconstant_unsigned(node->region(), nbits, elem_type_size);
	auto diff = jive_bitdifference(address1, address2);
	auto div = jive_bitsquotient(diff, elem_size);

	node->output(0)->replace(div);
}

static void
jive_apply_node_address_transform(const jive::node * node, size_t nbits)
{
	jive::input * fct = node->input(0);

	const jive::base::type * fcttype = &fct->type();
	if (!type_contains_address(fcttype))
		return;

	size_t narguments = node->ninputs()-1;

	size_t n;
	jive::oport * arguments[narguments];
	for (n = 1; n < node->ninputs(); n++) {
		jive::input * argument = node->input(n);
		arguments[n-1] = jive_address_to_bitstring_create(
			argument->origin(), nbits, &argument->type());
	}
	auto function = jive_address_to_bitstring_create(fct->origin(), nbits, fcttype);

	auto results = jive_apply_create(function, narguments, arguments);

	for (n = 0; n < results.size(); n++) {
		jive::oport * original = node->output(n);
		auto substitute = jive_bitstring_to_address_create(results[n], nbits, &original->type());
		original->replace(substitute);
	}
}

static void
jive_lambda_node_address_transform(
	const jive::node * node, size_t nbits)
{
	JIVE_DEBUG_ASSERT(node->noutputs() == 1);

	jive_graph * graph = node->graph();
	jive::output * fct = node->output(0);

	const jive::base::type * type = &fct->type();
	if (!type_contains_address(type))
		return;

	jive::region * region = node->input(0)->origin()->region();
	JIVE_DEBUG_ASSERT(node->region() != region);
	jive::node * enter = region->top();
	jive::node * leave = region->bottom();

	const jive::fct::type * fcttype = dynamic_cast<const jive::fct::type*>(type);
	std::unique_ptr<jive::base::type> new_fcttype_(convert_address_to_bitstring_type(*fcttype, nbits));
	jive::fct::type * new_fcttype = static_cast<jive::fct::type*>(new_fcttype_.get());

	size_t n;
	size_t nparameters = fcttype->narguments();
	const char * parameter_names[nparameters];
	for (n = 1; n < enter->noutputs(); n++)
		parameter_names[n-1] = enter->output(n)->gate()->name().c_str();

	const jive::base::type * argument_types[new_fcttype->narguments()];
	for (size_t i = 0; i < new_fcttype->narguments(); i++)
		argument_types[i] = new_fcttype->argument_type(i);

	jive_lambda * lambda = jive_lambda_begin(graph->root(), new_fcttype->narguments(),
		argument_types, parameter_names);

	jive::substitution_map map;

	jive::node * new_enter = lambda->region->top();
	for (n = 1; n < enter->noutputs(); n++) {
		jive::oport * parameter = jive_bitstring_to_address_create(new_enter->output(n), nbits,
			fcttype->argument_type(n-1));
		map.insert(enter->output(n), dynamic_cast<jive::output*>(parameter));
	}

	region->copy(lambda->region, map, false, false);

	size_t nresults = fcttype->nreturns();
	jive::oport * results[nresults];
	for (n = 1; n < leave->ninputs(); n++) {
		auto substitute = map.lookup(dynamic_cast<jive::output*>(leave->input(n)->origin()));
		results[n-1] = jive_address_to_bitstring_create(substitute, nbits, fcttype->return_type(n-1));
	}

	const jive::base::type * return_types[new_fcttype->nreturns()];
	for (size_t i = 0; i < new_fcttype->nreturns(); i++)
		return_types[i] = new_fcttype->return_type(i);

	auto new_fct = jive_lambda_end(lambda, new_fcttype->nreturns(), return_types, results);

	fct->replace(jive_bitstring_to_address_create(new_fct, nbits, type));
}

static void
jive_graph_tail_node_address_transform(const jive::node * node, size_t nbits)
{
	JIVE_DEBUG_ASSERT(dynamic_cast<const jive::graph_tail_operation*>(&node->operation()));

	jive_graph * graph = node->graph();

	bool transform = false;
	std::vector<std::string> names;
	std::vector<jive::oport*> exports;
	for (size_t n = 0; n < node->ninputs(); n++) {
		jive::output * origin = dynamic_cast<jive::output*>(node->input(n)->origin());
		if (type_contains_address(&origin->type())) {
			transform = true;
			names.push_back(node->input(n)->gate()->name());
			exports.push_back(jive_address_to_bitstring_create(origin, nbits, &origin->type()));
		} else {
			exports.push_back(origin);
			names.push_back(node->input(n)->gate()->name());
		}
	}

	if (!transform)
		return;

	delete graph->root()->bottom();
	jive::graph_tail_operation().create_node(graph->root(), {});

	JIVE_DEBUG_ASSERT(exports.size() == names.size());
	for (size_t n = 0; n < exports.size(); n++)
		graph->export_port(exports[n], names[n]);
}

void
jive_graph_address_transform(jive_graph * graph, jive::memlayout_mapper * mapper)
{
	for (jive::node * node : jive::topdown_traverser(graph)) {
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
	} else if (dynamic_cast<const jive::fct::lambda_op *>(&node->operation())) {
		jive_lambda_node_address_transform(node, nbits);
	} else if (dynamic_cast<const jive::graph_tail_operation*>(&node->operation())) {
		jive_graph_tail_node_address_transform(node, nbits);
	}
}

namespace jive {

address_to_bitstring_operation::~address_to_bitstring_operation() noexcept
{
}

address_to_bitstring_operation::address_to_bitstring_operation(
	size_t nbits,
	std::unique_ptr<jive::base::type> original_type)
	: nbits_(nbits)
	, result_type_(convert_address_to_bitstring_type(*original_type, nbits))
	, original_type_(std::move(original_type))
{}

jive_unop_reduction_path_t
address_to_bitstring_operation::can_reduce_operand(
	const jive::oport * arg) const noexcept
{
	auto op = dynamic_cast<const jive::output*>(arg);

	auto up = dynamic_cast<const bitstring_to_address_operation *>(&op->node()->operation());
	if (up) {
		if (up->nbits() != nbits())
			return jive_unop_reduction_none;

		if (up->original_type() != original_type())
			return jive_unop_reduction_none;

		return jive_unop_reduction_inverse;
	}

	if (!type_contains_address(&arg->type()))
		return jive_unop_reduction_idempotent;

	return jive_unop_reduction_none;
}

jive::oport *
address_to_bitstring_operation::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::oport * arg) const
{
	auto op = static_cast<jive::output*>(arg);

	if (path == jive_unop_reduction_inverse)
		return dynamic_cast<jive::output*>(op->node()->input(0)->origin());

	if (path == jive_unop_reduction_idempotent)
		return arg;

	return nullptr;
}

const jive::base::type &
address_to_bitstring_operation::argument_type(size_t index) const noexcept
{
	return *original_type_.get();
}

const jive::base::type &
address_to_bitstring_operation::result_type(size_t index) const noexcept
{
	return *result_type_.get();
}

bool
address_to_bitstring_operation::operator==(const operation & other) const noexcept
{
	const address_to_bitstring_operation * o =
		dynamic_cast<const address_to_bitstring_operation *>(&other);
	return o && nbits() == o->nbits();
}
std::string
address_to_bitstring_operation::debug_string() const
{
	return "ADDRESS_TO_BITSTRING";
}

std::unique_ptr<jive::operation>
address_to_bitstring_operation::copy() const
{
	return std::unique_ptr<jive::operation>(new address_to_bitstring_operation(*this));
}

bitstring_to_address_operation::~bitstring_to_address_operation() noexcept
{
}

bitstring_to_address_operation::bitstring_to_address_operation(
	size_t nbits,
	std::unique_ptr<jive::base::type> original_type)
	: nbits_(nbits)
	, argument_type_(convert_address_to_bitstring_type(*original_type, nbits))
	, original_type_(std::move(original_type))
{}

jive_unop_reduction_path_t
bitstring_to_address_operation::can_reduce_operand(
	const jive::oport * arg) const noexcept
{
	auto op = dynamic_cast<const jive::output*>(arg);
	if (!op)
		return jive_unop_reduction_none;

	auto up = dynamic_cast<const address_to_bitstring_operation *>(&op->node()->operation());
	if (up) {
		if (up->nbits() != nbits())
			return jive_unop_reduction_none;

		if (up->original_type() != original_type())
			return jive_unop_reduction_none;

		return jive_unop_reduction_inverse;
	}

	if (!type_contains_address(&original_type()))
		return jive_unop_reduction_idempotent;

	return jive_unop_reduction_none;
}

jive::oport *
bitstring_to_address_operation::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::oport * arg) const
{
	auto op = static_cast<jive::output*>(arg);

	if (path == jive_unop_reduction_inverse)
		return dynamic_cast<jive::output*>(op->node()->input(0)->origin());

	if (path == jive_unop_reduction_idempotent)
		return arg;

	return nullptr;
}

const jive::base::type &
bitstring_to_address_operation::argument_type(size_t index) const noexcept
{
	return *argument_type_.get();
}

const jive::base::type &
bitstring_to_address_operation::result_type(size_t index) const noexcept
{
	return *original_type_.get();
}

bool
bitstring_to_address_operation::operator==(const operation & other) const noexcept
{
	const bitstring_to_address_operation * o =
		dynamic_cast<const bitstring_to_address_operation *>(&other);
	return o && nbits() == o->nbits();
}
std::string
bitstring_to_address_operation::debug_string() const
{
	return "BITSTRING_TO_ADDRESS";
}

std::unique_ptr<jive::operation>
bitstring_to_address_operation::copy() const
{
	return std::unique_ptr<jive::operation>(new bitstring_to_address_operation(*this));
}

}
