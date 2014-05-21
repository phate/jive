/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/call.h>

#include <jive/arch/addresstype.h>
#include <jive/types/bitstring/type.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {

call_operation::~call_operation() noexcept {}

call_operation::call_operation(
	const jive_calling_convention * calling_convention,
	const std::vector<std::unique_ptr<jive_type>> & return_types)
	: calling_convention_(calling_convention)
{
	for (const std::unique_ptr<jive_type> & type : return_types) {
		return_types_.emplace_back(type->copy());
	}
}

call_operation::call_operation(const call_operation & other)
	: call_operation(other.calling_convention_, other.return_types_)
{
}

}

static void
jive_call_node_fini_(jive_node * self_);

static bool
jive_call_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_);

static jive_node *
jive_call_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_CALL_NODE = {
	parent : &JIVE_NODE,
	name : "CALL",
	fini : jive_call_node_fini_, /* override */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : jive_node_get_label_, /* inherit */
	match_attrs : jive_call_node_match_attrs_, /* override */
	check_operands : jive_node_check_operands_, /* inherit */
	create : jive_call_node_create_, /* override */
};

static void
jive_call_node_fini_(jive_node * self_)
{
	jive_node_fini_(self_);
}

static bool
jive_call_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_)
{
	const jive_call_node * self = (const jive_call_node *) self_;
	const jive::call_operation * attrs = (const jive::call_operation *) attrs_;
	
	if (self->operation().return_types().size() != attrs->return_types().size())
		return false;
	
	size_t n;
	for (n = 0; n < attrs->return_types().size(); n++) {
		if (*self->operation().return_types()[n] != *attrs->return_types()[n])
			return false;
	}
	
	return true;
}

static jive_node *
jive_call_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	const jive::call_operation * attrs = (const jive::call_operation *) attrs_;

	size_t nreturns = attrs->return_types().size();
	const jive_type * return_types[nreturns];
	for (size_t n = 0; n < nreturns; ++n) {
		return_types[n] = &*attrs->return_types()[n];
	}
	if (dynamic_cast<jive_bitstring_output*>(operands[0])){
		size_t nbits = jive_bitstring_output_nbits((const jive_bitstring_output *) operands[0]);
		return jive_call_by_bitstring_node_create(region, operands[0], nbits,
			attrs->calling_convention(), noperands - 1, operands + 1,
			nreturns, return_types);
	} else {
		return jive_call_by_address_node_create(region, operands[0], attrs->calling_convention(),
			noperands - 1, operands + 1,
			nreturns, return_types);
	}
}

void
jive_call_node_init_(jive_call_node * self,
	jive_region * region,
	jive_output * target_address, const jive_type * address_type,
	const jive_calling_convention * calling_convention,
	size_t narguments, jive_output * const arguments[],
	size_t nreturns, const jive_type * const return_types[])
{
	jive_context * context = region->graph->context;

	jive_output * operands[narguments + 1];
	const jive_type * operand_types[narguments + 1];

	size_t n;
	operands[0] = target_address;
	operand_types[0] = address_type;
	for (n = 0; n < narguments; n++) {
		operands[n + 1] = arguments[n];
		operand_types[n + 1] = &arguments[n]->type();
	}
	
	jive_node_init_(self, region,
		narguments + 1, operand_types, operands,
		nreturns, return_types);
}

static inline jive_region *
call_node_region_innermost(jive_output * target_address, size_t narguments,
	jive_output * const arguments[])
{
	size_t i;
	jive_output * tmp[narguments+1];
	for(i = 0; i < narguments; i++){
		tmp[i] = arguments[i];
	}
	tmp[i] = target_address;
	
	return jive_region_innermost(narguments + 1, tmp);
}

jive_node *
jive_call_by_address_node_create(jive_region * region,
	jive_output * target_address, const jive_calling_convention * calling_convention,
	size_t narguments, jive_output * const arguments[],
	size_t nreturns, const jive_type * const return_types[])
{
	std::vector<std::unique_ptr<jive_type>> return_types_tmp;
	for (size_t n = 0; n < nreturns; ++n) {
		return_types_tmp.emplace_back(return_types[n]->copy());
	}
	jive::call_operation op(calling_convention, return_types_tmp);

	jive_call_node * node = new jive_call_node(op);

	node->class_ = &JIVE_CALL_NODE;
	jive_address_type address_type;
	jive_call_node_init_(node, region, target_address, &address_type, calling_convention,
		narguments, arguments, nreturns, return_types);

	return node;
}

jive_output * const *
jive_call_by_address_create(jive_output * target_address,
	const jive_calling_convention * calling_convention,
	size_t narguments, jive_output * const arguments[],
	size_t nreturns, const jive_type * const return_types[])
{
	jive_region * region = call_node_region_innermost(target_address, narguments, arguments);

	return jive_call_by_address_node_create(region, target_address, calling_convention,
		narguments, arguments, nreturns, return_types)->outputs;
}

jive_node *
jive_call_by_bitstring_node_create(jive_region * region,
	jive_output * target_address, size_t nbits,
	const jive_calling_convention * calling_convention,
	size_t narguments, jive_output * const arguments[],
	size_t nreturns, const jive_type * const return_types[])
{
	std::vector<std::unique_ptr<jive_type>> return_types_tmp;
	for (size_t n = 0; n < nreturns; ++n) {
		return_types_tmp.emplace_back(return_types[n]->copy());
	}
	jive::call_operation op(calling_convention, return_types_tmp);
	jive_call_node * node = new jive_call_node(op);

	node->class_ = &JIVE_CALL_NODE;
	jive_bitstring_type address_type(nbits);
	jive_call_node_init_(node, region, target_address, &address_type, calling_convention,
		narguments, arguments, nreturns, return_types);

	return node;
}

jive_output * const *
jive_call_by_bitstring_create(jive_output * target_address, size_t nbits,
	const jive_calling_convention * calling_convention,
	size_t narguments, jive_output * const arguments[],
	size_t nreturns, const jive_type * const return_types[])
{
	jive_region * region = call_node_region_innermost(target_address, narguments, arguments);

	return jive_call_by_bitstring_node_create(region, target_address, nbits, calling_convention,
		narguments, arguments, nreturns, return_types)->outputs;
}
