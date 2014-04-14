/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/call.h>

#include <jive/arch/addresstype.h>
#include <jive/types/bitstring/type.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

static void
jive_call_node_fini_(jive_node * self_);

static const jive_node_attrs *
jive_call_node_get_attrs_(const jive_node * self_);

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
	get_attrs : jive_call_node_get_attrs_, /* override */
	match_attrs : jive_call_node_match_attrs_, /* override */
	check_operands : jive_node_check_operands_, /* inherit */
	create : jive_call_node_create_, /* override */
};

static void
jive_call_node_fini_(jive_node * self_)
{
	jive_context * context = self_->graph->context;
	jive_call_node * self = (jive_call_node *) self_;
	
	size_t n;
	for (n = 0; n < self->attrs.nreturns; n++) {
		jive_type_destroy(self->attrs.return_types[n]);
	}
	
	jive_context_free(context, self->attrs.return_types);
	
	jive_node_fini_(self);
}

static const jive_node_attrs *
jive_call_node_get_attrs_(const jive_node * self_)
{
	const jive_call_node * self = (const jive_call_node *) self_;
	return &self->attrs;
}

static bool
jive_call_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_)
{
	const jive_call_node * self = (const jive_call_node *) self_;
	const jive_call_node_attrs * attrs = (const jive_call_node_attrs *) attrs_;
	
	if (self->attrs.nreturns != attrs->nreturns)
		return false;
	
	size_t n;
	for (n = 0; n < attrs->nreturns; n++) {
		if (!jive_type_equals(self->attrs.return_types[n], attrs->return_types[n]))
			return false;
	}
	
	return true;
}

static jive_node *
jive_call_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	const jive_call_node_attrs * attrs = (const jive_call_node_attrs *) attrs_;

	if(jive_output_isinstance(operands[0], &JIVE_BITSTRING_OUTPUT)){
		size_t nbits = jive_bitstring_output_nbits((const jive_bitstring_output *) operands[0]);
		return jive_call_by_bitstring_node_create(region, operands[0], nbits,
			attrs->calling_convention, noperands - 1, operands + 1,
			attrs->nreturns, (const jive_type * const *) attrs->return_types);
	} else {
		return jive_call_by_address_node_create(region, operands[0], attrs->calling_convention,
			noperands - 1, operands + 1,
			attrs->nreturns, (const jive_type * const *) attrs->return_types);
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
		operand_types[n + 1] = jive_output_get_type(arguments[n]);
	}
	
	jive_node_init_(self, region,
		narguments + 1, operand_types, operands,
		nreturns, return_types);
	
	self->attrs.nreturns = nreturns;
	self->attrs.return_types = jive_context_malloc(context,
		sizeof(*self->attrs.return_types) * nreturns);
	
	for (n = 0; n < nreturns; n++)
		self->attrs.return_types[n] = jive_type_copy(return_types[n]);
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
	jive_call_node * node = new jive_call_node;

	node->class_ = &JIVE_CALL_NODE;
	JIVE_DECLARE_ADDRESS_TYPE(address_type);
	jive_call_node_init_(node, region, target_address, address_type, calling_convention,
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
		narguments, arguments, nreturns, return_types)->outputs ; 
}

jive_node *
jive_call_by_bitstring_node_create(jive_region * region,
	jive_output * target_address, size_t nbits,
	const jive_calling_convention * calling_convention,
	size_t narguments, jive_output * const arguments[],
	size_t nreturns, const jive_type * const return_types[])
{
	jive_call_node * node = new jive_call_node;

	node->class_ = &JIVE_CALL_NODE;
	JIVE_DECLARE_BITSTRING_TYPE(address_type, nbits);
	jive_call_node_init_(node, region, target_address, address_type, calling_convention,
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
		narguments, arguments, nreturns, return_types)->outputs ;	
}
