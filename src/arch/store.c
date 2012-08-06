#include <jive/arch/store.h>

#include <jive/vsdg/valuetype.h>
#include <jive/vsdg/node-private.h>
#include <jive/types/bitstring/type.h>
#include <jive/arch/addresstype.h>

static void
jive_store_node_fini_(jive_node * self_);

static const jive_node_attrs *
jive_store_node_get_attrs_(const jive_node * self_);

static bool
jive_store_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_);

static jive_node *
jive_store_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_STORE_NODE = {
	.parent = &JIVE_NODE,
	.name = "STORE",
	.fini = jive_store_node_fini_, /* override */
	.get_default_normal_form = jive_node_get_default_normal_form_, /* inherit */
	.get_label = jive_node_get_label_, /* inherit */
	.get_attrs = jive_store_node_get_attrs_, /* override */
	.match_attrs = jive_store_node_match_attrs_, /* override */
	.create = jive_store_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

static void
jive_store_node_fini_(jive_node * self_)
{
	jive_context * context = self_->graph->context;
	jive_store_node * self = (jive_store_node *) self_;
	
	jive_type_fini(&self->attrs.datatype->base);
	jive_context_free(context, self->attrs.datatype);
	
	jive_node_fini_(&self->base);
}

static const jive_node_attrs *
jive_store_node_get_attrs_(const jive_node * self_)
{
	const jive_store_node * self = (const jive_store_node *) self_;
	return &self->attrs.base;
}

static bool
jive_store_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_)
{
	const jive_store_node * self = (const jive_store_node *) self_;
	const jive_store_node_attrs * attrs = (const jive_store_node_attrs *) attrs_;
	return jive_type_equals(&self->attrs.datatype->base, &attrs->datatype->base);
}

static jive_node *
jive_store_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 2);
	const jive_store_node_attrs * attrs = (const jive_store_node_attrs *) attrs_;

	if(jive_output_isinstance(operands[0], &JIVE_BITSTRING_OUTPUT)){
		size_t nbits = jive_bitstring_output_nbits((const jive_bitstring_output *) operands[0]);
		return jive_store_by_bitstring_node_create(region, operands[0], nbits, attrs->datatype,
			operands[1], 0, NULL);
	} else {
		return jive_store_by_address_node_create(region, operands[0], attrs->datatype, operands[1],
			0, NULL);
	}
}

void
jive_store_node_init_(jive_store_node * self, jive_region * region,
	jive_output * address, const jive_type * address_type,
	const jive_value_type * datatype, jive_output * value,
	size_t nstates, jive_output * const states[])
{
	const jive_type * operand_types[2] = {address_type, &datatype->base};
	jive_output * operands[2] = {address, value};
	
	jive_node_init_(&self->base, region,
		2, operand_types, operands,
		0, NULL);
	self->attrs.datatype = (jive_value_type *) jive_type_copy(&datatype->base,
		region->graph->context);

	/* FIXME: check the type of the states */	
	size_t n;
	for (n = 0; n < nstates; n++) {
		const jive_type * type = jive_output_get_type(states[n]);
		jive_node_add_input(&self->base, type, states[n]);
		jive_node_add_output(&self->base, type);
	}
}

static inline jive_region *
store_node_region_innermost(jive_output * address, jive_output * value,
	size_t nstates, jive_output * const states[])
{
	size_t i;
	jive_output * outputs[nstates+2];
	for(i = 0; i < nstates; i++){
		outputs[i] = states[i];
	}
	outputs[nstates] = address;
	outputs[nstates+1] = value;
	
	return jive_region_innermost(nstates+2, outputs);
}

jive_node *
jive_store_by_address_node_create(jive_region * region, jive_output * address,
	const jive_value_type * datatype, jive_output * value,
	size_t nstates, jive_output * const states[])
{
	jive_store_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	
	node->base.class_ = &JIVE_STORE_NODE;
	JIVE_DECLARE_ADDRESS_TYPE(address_type);
	jive_store_node_init_(node, region, address, address_type, datatype, value, nstates, states);
	
	return &node->base;
}

jive_output * const *
jive_store_by_address_create(jive_output * address,
	const jive_value_type * datatype, jive_output * value,
	size_t nstates, jive_output * const states[])
{
	jive_region * region = store_node_region_innermost(address, value, nstates, states);
	jive_node * node = jive_store_by_address_node_create(region, address, datatype, value,
		nstates, states);

	return node->outputs;
}

jive_node *
jive_store_by_bitstring_node_create(jive_region * region,
	jive_output * address, size_t nbits,
	const jive_value_type * datatype, jive_output * value,
	size_t nstates, jive_output * const states[])
{
	jive_store_node * node = jive_context_malloc(region->graph->context, sizeof(*node));

	node->base.class_ = &JIVE_STORE_NODE;
	JIVE_DECLARE_BITSTRING_TYPE(address_type, nbits);
	jive_store_node_init_(node, region, address, address_type, datatype, value, nstates, states);

	return &node->base;
}

jive_output * const *
jive_store_by_bitstring_create(jive_output * address, size_t nbits,
	const jive_value_type * datatype, jive_output * value,
	size_t nstates, jive_output * const states[])
{
	jive_region * region = store_node_region_innermost(address, value, nstates, states);
	jive_node * node = jive_store_by_bitstring_node_create(region, address, nbits, datatype, value,
		nstates, states);

	return node->outputs;
}
