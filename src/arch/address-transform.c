#include <jive/arch/address-transform.h>

#include <jive/arch/addresstype.h>
#include <jive/arch/memlayout.h>
#include <jive/bitstring/type.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

/* address_to_bitstring node */

static void
jive_address_to_bitstring_node_init_(jive_address_to_bitstring_node * self, jive_region * region,
	jive_output * address, size_t nbits);

static const jive_node_attrs *
jive_address_to_bitstring_node_get_attrs_(const jive_node * self);

static bool
jive_address_to_bitstring_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs);

static jive_node *
jive_address_to_bitstring_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

static bool
jive_address_to_bitstring_node_can_reduce_operand_(const jive_node_class * cls,
	const jive_node_attrs * attrs, jive_output * operand);

static bool
jive_address_to_bitstring_node_reduce_operand_(const jive_node_class * cls,
	const jive_node_attrs * attrs, jive_output ** operand);

const jive_unary_operation_class JIVE_ADDRESS_TO_BITSTRING_NODE_ = {
	.base = { /* jive_node_class */
		.parent = &JIVE_UNARY_OPERATION,
		.name = "ADDRESS_TO_BITSTRING",
		.fini = jive_node_fini_, /* inherit */
		.get_label = jive_node_get_label_, /* inherit */
		.get_attrs = jive_address_to_bitstring_node_get_attrs_, /* override */
		.match_attrs = jive_address_to_bitstring_node_match_attrs_, /* override */
		.create = jive_address_to_bitstring_node_create_, /* override */
		.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
	},

	.single_apply_over = NULL,
	.multi_apply_over = NULL,
	
	.can_reduce_operand = jive_address_to_bitstring_node_can_reduce_operand_, /* override */
	.reduce_operand = jive_address_to_bitstring_node_reduce_operand_ /* override */
};

static void
jive_address_to_bitstring_node_init_(
	jive_address_to_bitstring_node * self,
	jive_region * region,
	jive_output * address,
	size_t nbits)
{
	JIVE_DECLARE_ADDRESS_TYPE(addrtype);
	JIVE_DECLARE_BITSTRING_TYPE(btype, nbits);

	jive_node_init_(&self->base, region,
		1, &addrtype, &address,
		1, &btype);

	self->attrs.nbits = nbits;
}

static const jive_node_attrs *
jive_address_to_bitstring_node_get_attrs_(const jive_node * self_)
{
	const jive_address_to_bitstring_node * self = (const jive_address_to_bitstring_node *) self_;
	
	return &self->attrs.base;
}

static bool
jive_address_to_bitstring_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_address_to_bitstring_node_attrs * first = &((const jive_address_to_bitstring_node *)
		self)->attrs;
	const jive_address_to_bitstring_node_attrs * second =
		(const jive_address_to_bitstring_node_attrs *) attrs;
	
	return (first->nbits == second->nbits);
}

static jive_node *
jive_address_to_bitstring_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);

	const jive_address_to_bitstring_node_attrs * attrs =
		(const jive_address_to_bitstring_node_attrs *) attrs_;

	jive_address_to_bitstring_node * node = jive_context_malloc(region->graph->context,
		sizeof(*node));
	node->base.class_ = &JIVE_ADDRESS_TO_BITSTRING_NODE;
	jive_address_to_bitstring_node_init_(node, region, operands[0], attrs->nbits);

	return &node->base;
}

static bool
jive_address_to_bitstring_node_can_reduce_operand_(const jive_node_class * cls,
	const jive_node_attrs * attrs_, jive_output * operand_)
{

	const jive_address_to_bitstring_node_attrs * attrs =
		(const jive_address_to_bitstring_node_attrs *) attrs_;

	if(jive_node_isinstance(operand_->node, &JIVE_BITSTRING_TO_ADDRESS_NODE)){
		const jive_bitstring_to_address_node * node = (const jive_bitstring_to_address_node *) node;
		JIVE_DEBUG_ASSERT(node->attrs.nbits == attrs->nbits);
		return true;
	}

	return false;	
}

static bool
jive_address_to_bitstring_node_reduce_operand_(const jive_node_class * cls,
	const jive_node_attrs * attrs_, jive_output ** operand_)
{

	if(jive_node_isinstance(operand_[0]->node, &JIVE_BITSTRING_TO_ADDRESS_NODE)){
		operand_[0] = operand_[0]->node->inputs[0]->origin;
		return true;
	}

	return false;
}

jive_node *
jive_address_to_bitstring_node_create(struct jive_region * region,
	jive_output * address, size_t nbits)
{
	jive_address_to_bitstring_node_attrs attrs;
	attrs.nbits = nbits; 

	return jive_unary_operation_normalized_create(&JIVE_ADDRESS_TO_BITSTRING_NODE, region,
		&attrs.base, address)->node; 
}

jive_output *
jive_address_to_bitstring_create(jive_output * address, size_t nbits)
{
	jive_address_to_bitstring_node_attrs attrs;
	attrs.nbits = nbits; 

	return jive_unary_operation_normalized_create(&JIVE_ADDRESS_TO_BITSTRING_NODE,
		address->node->region, &attrs.base, address);
}


/* bitstring_to_address node */

static void
jive_bitstring_to_address_node_init_(jive_bitstring_to_address_node * self, jive_region * region,
	jive_output * bitstring, size_t nbits);

static const jive_node_attrs *
jive_bitstring_to_address_node_get_attrs_(const jive_node * self);

static bool
jive_bitstring_to_address_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs);

static jive_node *
jive_bitstring_to_address_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

static bool
jive_bitstring_to_address_node_can_reduce_operand_(const jive_node_class * cls,
	const jive_node_attrs * attrs, jive_output * operand);

static bool
jive_bitstring_to_address_node_reduce_operand_(const jive_node_class * cls,
	const jive_node_attrs * attrs, jive_output ** operand);

const jive_unary_operation_class JIVE_BITSTRING_TO_ADDRESS_NODE_ = {
	.base = { /* jive_node_class */
		.parent = &JIVE_UNARY_OPERATION,
		.name = "BITSTRING_TO_ADDRESS",
		.fini = jive_node_fini_, /* inherit */
		.get_label = jive_node_get_label_, /* inherit */
		.get_attrs = jive_bitstring_to_address_node_get_attrs_, /* override */
		.match_attrs = jive_bitstring_to_address_node_match_attrs_, /* override */
		.create = jive_bitstring_to_address_node_create_, /* override */
		.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
	},

	.single_apply_over = NULL,
	.multi_apply_over = NULL,

	.can_reduce_operand = jive_bitstring_to_address_node_can_reduce_operand_, /* override */
	.reduce_operand = jive_bitstring_to_address_node_reduce_operand_ /* override */	
};

static void
jive_bitstring_to_address_node_init_(
	jive_bitstring_to_address_node * self,
	jive_region * region,
	jive_output * bitstring,
	size_t nbits)
{
	JIVE_DECLARE_BITSTRING_TYPE(btype, nbits);
	JIVE_DECLARE_ADDRESS_TYPE(addrtype);

	jive_node_init_(&self->base, region,
		1, &btype, &bitstring,
		1, &addrtype);

	self->attrs.nbits = nbits;
}

static const jive_node_attrs *
jive_bitstring_to_address_node_get_attrs_(const jive_node * self_)
{
	const jive_bitstring_to_address_node * self = (const jive_bitstring_to_address_node *) self_;

	return &self->attrs.base;
}

static bool
jive_bitstring_to_address_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_bitstring_to_address_node_attrs * first = &((const jive_bitstring_to_address_node *)
		self)->attrs;
	const jive_bitstring_to_address_node_attrs * second =
		(const jive_bitstring_to_address_node_attrs *) attrs;

	return (first->nbits == second->nbits);
}

static jive_node *
jive_bitstring_to_address_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);

	const jive_bitstring_to_address_node_attrs * attrs =
		(const jive_bitstring_to_address_node_attrs *) attrs_;

	jive_bitstring_to_address_node * node = jive_context_malloc(region->graph->context,
		sizeof(*node));
	node->base.class_ = &JIVE_BITSTRING_TO_ADDRESS_NODE;
	jive_bitstring_to_address_node_init_(node, region, operands[0], attrs->nbits);

	return &node->base; 
}

static bool
jive_bitstring_to_address_node_can_reduce_operand_(const jive_node_class * cls,
	const jive_node_attrs * attrs_, jive_output * operand_)
{
	const jive_bitstring_to_address_node_attrs * attrs =
		(const jive_bitstring_to_address_node_attrs *) attrs_;

	if(jive_node_isinstance(operand_->node, &JIVE_ADDRESS_TO_BITSTRING_NODE)){
		const jive_address_to_bitstring_node * node = (const jive_address_to_bitstring_node *) node;
		JIVE_DEBUG_ASSERT(node->attrs.nbits == attrs->nbits);
		return true;
	}

	return false;
}

static bool
jive_bitstring_to_address_node_reduce_operand_(const jive_node_class * cls,
	const jive_node_attrs * attrs_, jive_output ** operand_)
{
	if(jive_node_isinstance(operand_[0]->node, &JIVE_ADDRESS_TO_BITSTRING_NODE)){
		operand_[0] = operand_[0]->node->inputs[0]->origin;
		return true;
	}

	return false;
}

jive_node *
jive_bitstring_to_address_node_create(struct jive_region * region,
	jive_output * bitstring, size_t nbits)
{
	jive_bitstring_to_address_node_attrs attrs;
	attrs.nbits = nbits; 

	return jive_unary_operation_normalized_create(&JIVE_BITSTRING_TO_ADDRESS_NODE, region,
		&attrs.base, bitstring)->node;
}

jive_output *
jive_bitstring_to_address_create(jive_output * bitstring, size_t nbits)
{
	jive_bitstring_to_address_node_attrs attrs;
	attrs.nbits = nbits;

	return jive_unary_operation_normalized_create(&JIVE_BITSTRING_TO_ADDRESS_NODE,
		bitstring->node->region, &attrs.base, bitstring);
}

