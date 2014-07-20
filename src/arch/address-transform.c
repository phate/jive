/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/address-transform.h>

#include <jive/arch/address.h>
#include <jive/arch/addresstype.h>
#include <jive/arch/call.h>
#include <jive/arch/load.h>
#include <jive/arch/memlayout.h>
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

static jive::base::type *
convert_address_to_bitstring_type(
	const jive::base::type * type,
	size_t nbits,
	jive_context * context)
{
	if (dynamic_cast<const jive::addr::type*>(type)) {
		jive::bits::type bittype(nbits);
		return bittype.copy();
	}

	const jive::fct::type * fcttype = dynamic_cast<const jive::fct::type*>(type);
	if (fcttype != NULL) {

		size_t n;
		size_t narguments = fcttype->narguments();
		const jive::base::type * argument_types[narguments];
		for (n = 0; n < narguments; n++)
			argument_types[n] = convert_address_to_bitstring_type(fcttype->argument_type(n), nbits,
				context);

		size_t nresults = fcttype->nreturns();
		const jive::base::type * result_types[nresults];
		for (n = 0; n < nresults; n++)
			result_types[n] = convert_address_to_bitstring_type(fcttype->return_type(n), nbits, context);

		jive::fct::type return_type(narguments, argument_types, nresults, result_types);

		jive::base::type * new_fcttype = return_type.copy();

		for (n = 0; n < narguments; n++)
			delete argument_types[n];
		for (n = 0; n < nresults; n++)
			delete result_types[n];

		return new_fcttype;
	}

	JIVE_DEBUG_ASSERT(0);
}

/* address_to_bitstring node */

static void
jive_address_to_bitstring_node_init_(jive_address_to_bitstring_node * self, jive_region * region,
	jive::output * address, size_t nbits, jive::base::type * original_type);

const jive_node_class JIVE_ADDRESS_TO_BITSTRING_NODE = {
	parent : &JIVE_UNARY_OPERATION,
	name : "ADDRESS_TO_BITSTRING",
	fini : jive_node_fini_, /* override */
	get_default_normal_form : jive_unary_operation_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

static void
jive_address_to_bitstring_node_init_(
	jive_address_to_bitstring_node * self,
	jive_region * region,
	jive::output * address,
	size_t nbits,	jive::base::type * original_type)
{
	jive_context * context = region->graph->context;

	const jive::base::type * addrtype = &address->type();
	if (!dynamic_cast<const jive::value::type*>(addrtype))
		jive_context_fatal_error(context, "Type mismatch: expected a value type.");

	JIVE_DEBUG_ASSERT(*addrtype == *original_type);
	jive::base::type * return_type = convert_address_to_bitstring_type(addrtype, nbits, context);

	jive_node_init_(self, region,
		1, &addrtype, &address,
		1, (const jive::base::type **)&return_type);

	delete return_type;
}

jive::output *
jive_address_to_bitstring_create(jive::output * address, size_t nbits,
	const jive::base::type * original_type)
{
	jive::address_to_bitstring_operation op(nbits, original_type);

	return jive_unary_operation_create_normalized(&JIVE_ADDRESS_TO_BITSTRING_NODE,
		address->node()->graph, &op, address);
}

/* bitstring_to_address node */

static void
jive_bitstring_to_address_node_init_(jive_bitstring_to_address_node * self, jive_region * region,
	jive::output * bitstring, size_t nbits, jive::base::type * original_type);

const jive_node_class JIVE_BITSTRING_TO_ADDRESS_NODE= {
	parent : &JIVE_UNARY_OPERATION,
	name : "BITSTRING_TO_ADDRESS",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_unary_operation_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

static void
jive_bitstring_to_address_node_init_(
	jive_bitstring_to_address_node * self,
	jive_region * region,
	jive::output * bitstring,
	size_t nbits,
	const jive::base::type * original_type)
{
	jive_context * context = region->graph->context;

	const jive::base::type * bittype = &bitstring->type();
	if (!dynamic_cast<const jive::value::type*>(bittype))
		jive_context_fatal_error(context, "Type mismatch: expected a value type.");

	jive_node_init_(self, region,
		1, &bittype, &bitstring,
		1, (const jive::base::type **)&original_type);
}

jive::output *
jive_bitstring_to_address_create(jive::output * bitstring, size_t nbits,
	const jive::base::type * original_type)
{
	jive::bitstring_to_address_operation op(nbits, original_type);

	return jive_unary_operation_create_normalized(&JIVE_BITSTRING_TO_ADDRESS_NODE,
		bitstring->node()->graph, &op, bitstring);
}

/* reductions */

void
jive_load_node_address_transform(
	jive_load_node * node,
	const jive::load_op & op,
	size_t nbits)
{
	bool input_is_address = dynamic_cast<jive::addr::input*>(node->inputs[0]);
	bool output_is_address = dynamic_cast<jive::addr::output*>(node->outputs[0]);

	if (!input_is_address && !output_is_address)
		return;

	jive::output * address = node->inputs[0]->origin();
	if (input_is_address)
		address = jive_address_to_bitstring_create(address, nbits, &address->type());

	JIVE_DEBUG_ASSERT(static_cast<const jive::bits::output*>(address)->nbits() == nbits);

	jive::bits::type bits(nbits);
	const jive::value::type * datatype = &op.data_type();
	if (output_is_address)
		datatype = &bits;

	size_t i;
	size_t nstates = node->ninputs - 1;
	jive::output * states[nstates];
	for (i = 0; i < nstates; i++){
		states[i] = node->inputs[i+1]->origin();
	}

	jive::output * load = jive_load_by_bitstring_create(address, nbits, datatype, nstates, states);
	
	if (output_is_address)
		load = jive_bitstring_to_address_create(load, nbits, &node->outputs[0]->type());
	
	jive_output_replace(node->outputs[0], load);
}

void
jive_store_node_address_transform(
	jive_node * node,
	const jive::store_op & op,
	size_t nbits)
{
	bool input0_is_address = dynamic_cast<jive::addr::input*>(node->inputs[0]);
	bool input1_is_address = dynamic_cast<jive::addr::input*>(node->inputs[1]);

	if (!input0_is_address && !input1_is_address)
		return;

	jive::output * address = node->inputs[0]->origin();
	if (input0_is_address)
		address = jive_address_to_bitstring_create(address, nbits, &address->type());

	JIVE_DEBUG_ASSERT(static_cast<const jive::bits::output*>(address)->nbits() == nbits);

	jive::bits::type bits(nbits);
	const jive::value::type * datatype = &op.data_type();
	jive::output * value = node->inputs[1]->origin();
	if (input1_is_address){
		datatype = &bits;
		value = jive_address_to_bitstring_create(node->inputs[1]->origin(), nbits, &value->type());
	}

	size_t nstates = node->ninputs - 2;
	jive::output * states[nstates];
	for (size_t n = 0; n < nstates; ++n){
		states[n] = node->inputs[n+2]->origin();
	}

	std::vector<jive::output *> ostates = jive_store_by_bitstring_create(
		address, nbits, datatype, value, nstates, states);

	for (size_t n = 0; n < nstates; ++n) {
		jive_output_replace(node->outputs[n], ostates[n]);
	}
}

void
jive_label_to_address_node_address_transform(jive_label_to_address_node * node, size_t nbits)
{
	const jive_label * label = node->operation().label();

	jive::output * label_o = jive_label_to_bitstring_create(node->region->graph, label, nbits);
	jive::output * addr_o = jive_bitstring_to_address_create(label_o, nbits,
		&node->outputs[0]->type());
	jive_output_replace(node->outputs[0], addr_o);
}

void
jive_call_node_address_transform(jive_call_node * node, size_t nbits)
{
	const jive_node * node_ = node;

	size_t i;
	bool transform = false;
	jive::output * operands[node_->ninputs];
	for (i = 0; i < node_->ninputs; i++){
		if(dynamic_cast<jive::addr::input*>(node_->inputs[i])){
			operands[i] = jive_address_to_bitstring_create(node_->inputs[i]->origin(), nbits,
				&node->inputs[i]->origin()->type());
			transform = true;
		} else {
			operands[i] = node_->inputs[i]->origin();
		}
	}

	const jive::base::type * return_types[node_->noutputs];
	jive::bits::type address_type(nbits);
	for (i = 0; i < node_->noutputs; i++){
		if (dynamic_cast<jive::addr::output*>(node_->outputs[i])){
			return_types[i] = &address_type;
			transform = true;
		} else {
			return_types[i] = &node->outputs[i]->type();
		}
	}

	if (!transform)
		return;

	jive_node * call = jive_call_by_bitstring_node_create(node_->region, operands[0], nbits,
		node->operation().calling_convention(), node_->ninputs - 1, operands + 1, node_->noutputs,
		return_types);

	for (i = 0; i < node_->noutputs; i++){
		jive::output * output = call->outputs[i];
		if(dynamic_cast<jive::addr::output*>(node_->outputs[i]))
			output = jive_bitstring_to_address_create(call->outputs[i], nbits, &node->outputs[i]->type());
		jive_output_replace(node_->outputs[i], output);
	}
}

void
jive_memberof_node_address_transform(jive_memberof_node * node, jive_memlayout_mapper * mapper)
{
	const jive_node * node_ = node;

	size_t index = node->operation().index();
	const jive::rcd::declaration * decl = node->operation().record_decl();

	JIVE_DEBUG_ASSERT(index < decl->nelements);
	size_t elem_offset = jive_memlayout_mapper_map_record(mapper, decl)->element[index].offset;
	size_t nbits = jive_memlayout_mapper_map_address(mapper)->total_size * 8;

	jive::output * offset = jive_bitconstant_unsigned(node_->graph, nbits, elem_offset);
	jive::output * address = jive_address_to_bitstring_create(node_->inputs[0]->origin(), nbits,
		&node->inputs[0]->origin()->type());
	jive::output * tmparray0[] = {address, offset};
	jive::output * sum = jive_bitsum(2, tmparray0);
	jive::output * off_address = jive_bitstring_to_address_create(sum, nbits,
		&node->outputs[0]->type());

	jive_output_replace(node_->outputs[0], off_address);
}

void
jive_containerof_node_address_transform(
	jive_containerof_node * node, jive_memlayout_mapper * mapper)
{
	const jive_node * node_ = node;

	size_t index = node->operation().index();
	const jive::rcd::declaration * decl = node->operation().record_decl();

	JIVE_DEBUG_ASSERT(index < decl->nelements);
	size_t elem_offset = jive_memlayout_mapper_map_record(mapper, decl)->element[index].offset;
	size_t nbits = jive_memlayout_mapper_map_address(mapper)->total_size * 8;

	jive::output * offset = jive_bitconstant_unsigned(node_->graph, nbits, elem_offset);
	jive::output * address = jive_address_to_bitstring_create(node_->inputs[0]->origin(), nbits,
		&node->inputs[0]->origin()->type());
	jive::output * sum = jive_bitdifference(address, offset);
	jive::output * off_address = jive_bitstring_to_address_create(sum, nbits,
		&node->outputs[0]->type());

	jive_output_replace(node_->outputs[0], off_address);
}

void
jive_arraysubscript_node_address_transform(jive_arraysubscript_node * node,
	struct jive_memlayout_mapper * mapper)
{
	const jive_node * node_ = node;

	size_t elem_type_size = jive_memlayout_mapper_map_value_type(mapper,
		&node->operation().element_type())->total_size;
	size_t nbits = jive_memlayout_mapper_map_address(mapper)->total_size * 8;

	jive::output * index = node_->inputs[1]->origin();
	jive::output * address = jive_address_to_bitstring_create(node_->inputs[0]->origin(), nbits,
		&node->inputs[0]->origin()->type());
	jive::output * elem_size = jive_bitconstant_unsigned(node_->graph, nbits, elem_type_size);
	jive::output * tmparray1[] = {elem_size, index};
	jive::output * offset = jive_bitmultiply(2, tmparray1);
	jive::output * tmparray2[] = {address, offset};
	jive::output * sum = jive_bitsum(2, tmparray2);
	jive::output * off_address = jive_bitstring_to_address_create(sum, nbits,
		&node->outputs[0]->type());
	
	jive_output_replace(node_->outputs[0], off_address);
}

void
jive_arrayindex_node_address_transform(jive_arrayindex_node * node, jive_memlayout_mapper * mapper)
{
	const jive_node * node_ = node;

	size_t elem_type_size = jive_memlayout_mapper_map_value_type(mapper,
		&node->operation().element_type())->total_size;
	size_t nbits = jive_memlayout_mapper_map_address(mapper)->total_size * 8;

	jive::output * address1 = jive_address_to_bitstring_create(node_->inputs[0]->origin(), nbits,
		&node->inputs[0]->origin()->type());
	jive::output * address2 = jive_address_to_bitstring_create(node_->inputs[1]->origin(), nbits,
		&node->inputs[1]->origin()->type());
	jive::output * elem_size = jive_bitconstant_unsigned(node_->graph, nbits, elem_type_size);
	jive::output * diff = jive_bitdifference(address1, address2);
	jive::output * div = jive_bitsquotient(diff, elem_size);

	jive_output_replace(node_->outputs[0], div);
}

void
jive_apply_node_address_transform(const jive_apply_node * node, size_t nbits)
{
	jive::input * fct = node->inputs[0];

	const jive::base::type * fcttype = &fct->type();
	if (!type_contains_address(fcttype))
		return;

	size_t nresults = node->noutputs;
	size_t narguments = node->ninputs-1;

	size_t n;
	jive::output * arguments[narguments];
	for (n = 1; n < node->ninputs; n++) {
		jive::input * argument = node->inputs[n];
		arguments[n-1] = jive_address_to_bitstring_create(argument->origin(), nbits, &argument->type());
	}
	jive::output * function = jive_address_to_bitstring_create(fct->origin(), nbits, fcttype);

	std::vector<jive::output *> results = jive_apply_create(function, narguments, arguments);

	for (n = 0; n < results.size(); n++) {
		jive::output * original = node->outputs[n];
		jive::output * substitute = jive_bitstring_to_address_create(results[n], nbits,
			&original->type());
		jive_output_replace(original, substitute);
	}
}

void
jive_lambda_node_address_transform(const jive_lambda_node * node, size_t nbits)
{
	JIVE_DEBUG_ASSERT(node->noutputs == 1);

	jive_graph * graph = node->graph;
	jive_context * context = graph->context;
	jive::output * fct = node->outputs[0];

	const jive::base::type * type = &fct->type();
	if (!type_contains_address(type))
		return;

	jive_node * enter = jive_lambda_node_get_enter_node(node);
	jive_node * leave = jive_lambda_node_get_leave_node(node);
	jive_region * region = jive_lambda_node_get_region(node);

	const jive::fct::type * fcttype = dynamic_cast<const jive::fct::type*>(type);
	jive::fct::type * new_fcttype = static_cast<jive::fct::type*>(convert_address_to_bitstring_type(
		fcttype, nbits, context));

	size_t n;
	size_t nparameters = fcttype->narguments();
	const char * parameter_names[nparameters];
	for (n = 1; n < enter->noutputs; n++)
		parameter_names[n-1] = enter->outputs[n]->gate->name;

	const jive::base::type * argument_types[new_fcttype->narguments()];
	for (size_t i = 0; i < new_fcttype->narguments(); i++)
		argument_types[i] = new_fcttype->argument_type(i);

	jive_lambda * lambda = jive_lambda_begin(graph, new_fcttype->narguments(),
		argument_types, parameter_names);

	jive_substitution_map * map = jive_substitution_map_create(context);

	jive_node * new_enter = jive_region_get_top_node(lambda->region);
	for (n = 1; n < enter->noutputs; n++) {
		jive::output * parameter = jive_bitstring_to_address_create(new_enter->outputs[n], nbits,
			fcttype->argument_type(n-1));
		jive_substitution_map_add_output(map, enter->outputs[n], parameter);
	}

	jive_region_copy_substitute(region, lambda->region, map, false, false);

	size_t nresults = fcttype->nreturns();
	jive::output * results[nresults];
	for (n = 1; n < leave->ninputs; n++) {
		jive::output * substitute = jive_substitution_map_lookup_output(map, leave->inputs[n]->origin());
		results[n-1] = jive_address_to_bitstring_create(substitute, nbits, fcttype->return_type(n-1));
	}

	jive_substitution_map_destroy(map);

	const jive::base::type * return_types[new_fcttype->nreturns()];
	for (size_t i = 0; i < new_fcttype->nreturns(); i++)
		return_types[i] = new_fcttype->return_type(i);

	jive::output * new_fct = jive_lambda_end(lambda, new_fcttype->nreturns(), return_types, results);

	delete new_fcttype;
	jive_output_replace(fct, jive_bitstring_to_address_create(new_fct, nbits, type));
}

static void
convert_regions(const struct jive_region * region, jive_memlayout_mapper * mapper)
{
	jive_region * subregion;
	JIVE_LIST_ITERATE(region->subregions, subregion, region_subregions_list)
		convert_regions(subregion, mapper);

	jive_node * anchor = jive_region_get_anchor_node(region);
	if (anchor == NULL)
		return;

	size_t nbits = jive_memlayout_mapper_map_address(mapper)->total_size * 8;

	const jive_lambda_node * lambda = jive_lambda_node_const_cast(anchor);
	if (lambda != NULL)
		jive_lambda_node_address_transform(lambda, nbits);
}

void
jive_graph_address_transform(jive_graph * graph, jive_memlayout_mapper * mapper)
{
	convert_regions(graph->root_region, mapper);

	jive_traverser * traverser = jive_topdown_traverser_create(graph);
	size_t nbits = jive_memlayout_mapper_map_address(mapper)->total_size * 8;

	jive_node * node = jive_traverser_next(traverser);
	for(; node; node = jive_traverser_next(traverser)){
		if (jive_node_isinstance(node, &JIVE_MEMBEROF_NODE))
			jive_memberof_node_address_transform(jive_memberof_node_cast(node), mapper);
		else if (jive_node_isinstance(node, &JIVE_CONTAINEROF_NODE))
			jive_containerof_node_address_transform(jive_containerof_node_cast(node), mapper);
		else if (jive_node_isinstance(node, &JIVE_ARRAYINDEX_NODE))
			jive_arrayindex_node_address_transform(jive_arrayindex_node_cast(node), mapper);
		else if (jive_node_isinstance(node, &JIVE_ARRAYSUBSCRIPT_NODE))
			jive_arraysubscript_node_address_transform(jive_arraysubscript_node_cast(node), mapper);
		else if (jive_node_isinstance(node, &JIVE_LABEL_TO_ADDRESS_NODE))
			jive_label_to_address_node_address_transform(jive_label_to_address_node_cast(node), nbits);
		else if (auto op = dynamic_cast<const jive::load_op *>(&node->operation())) {
			jive_load_node_address_transform(node, *op, nbits);
		} else if (auto op = dynamic_cast<const jive::store_op *>(&node->operation())) {
			jive_store_node_address_transform(node, *op, nbits);
		} else if (jive_node_isinstance(node, &JIVE_CALL_NODE))
			jive_call_node_address_transform(jive_call_node_cast(node), nbits);

		const jive_apply_node * apply_node = dynamic_cast<const jive_apply_node *>(node);
		if (apply_node) {
			jive_apply_node_address_transform(apply_node, nbits);
		}
	}

	jive_traverser_destroy(traverser);
}

namespace jive {

address_to_bitstring_operation::~address_to_bitstring_operation() noexcept
{
}

jive_unop_reduction_path_t
address_to_bitstring_operation::can_reduce_operand(
	const jive::output * arg) const noexcept
{
	const bitstring_to_address_operation * up =
		dynamic_cast<const bitstring_to_address_operation *>(&arg->node()->operation());
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

jive::output *
address_to_bitstring_operation::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	if (path == jive_unop_reduction_inverse)
		return arg->node()->inputs[0]->origin();

	if (path == jive_unop_reduction_idempotent)
		return arg;

	return NULL;
}

const jive::base::type &
address_to_bitstring_operation::argument_type(size_t index) const noexcept
{
	static const jive::addr::type type;
	return type;
}

const jive::base::type &
address_to_bitstring_operation::result_type(size_t index) const noexcept
{
	return result_type_;
}

bool
address_to_bitstring_operation::operator==(const operation & other) const noexcept
{
	const address_to_bitstring_operation * o =
		dynamic_cast<const address_to_bitstring_operation *>(&other);
	return o && nbits() == o->nbits();
}

jive_node *
address_to_bitstring_operation::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	JIVE_DEBUG_ASSERT(narguments == 1);

	jive_address_to_bitstring_node * node = new jive_address_to_bitstring_node(*this);
	node->class_ = &JIVE_ADDRESS_TO_BITSTRING_NODE;
	jive_address_to_bitstring_node_init_(node, region, arguments[0], nbits(),
		&original_type());

	return node;
}

std::string
address_to_bitstring_operation::debug_string() const
{
	return "ADDRESS_TO_BITSTRING";
}

bitstring_to_address_operation::~bitstring_to_address_operation() noexcept
{
}

jive_unop_reduction_path_t
bitstring_to_address_operation::can_reduce_operand(
	const jive::output * arg) const noexcept
{
	const address_to_bitstring_operation * up =
		dynamic_cast<const address_to_bitstring_operation *>(&arg->node()->operation());

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

jive::output *
bitstring_to_address_operation::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	if (path == jive_unop_reduction_inverse)
		return arg->node()->inputs[0]->origin();

	if (path == jive_unop_reduction_idempotent)
		return arg;

	return NULL;
}

const jive::base::type &
bitstring_to_address_operation::argument_type(size_t index) const noexcept
{
	return argument_type_;
}

const jive::base::type &
bitstring_to_address_operation::result_type(size_t index) const noexcept
{
	static const jive::addr::type type;
	return type;
}

bool
bitstring_to_address_operation::operator==(const operation & other) const noexcept
{
	const bitstring_to_address_operation * o =
		dynamic_cast<const bitstring_to_address_operation *>(&other);
	return o && nbits() == o->nbits();
}

jive_node *
bitstring_to_address_operation::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	JIVE_DEBUG_ASSERT(narguments == 1);

	jive_bitstring_to_address_node * node = new jive_bitstring_to_address_node(*this);
	node->class_ = &JIVE_BITSTRING_TO_ADDRESS_NODE;
	jive_bitstring_to_address_node_init_(node, region, arguments[0], nbits(),
		&original_type());

	return node;
}

std::string
bitstring_to_address_operation::debug_string() const
{
	return "BITSTRING_TO_ADDRESS";
}

}
