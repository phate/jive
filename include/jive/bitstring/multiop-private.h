#ifndef JIVE_BITSTRING_MULTIOP_PRIVATE_H
#define JIVE_BITSTRING_MULTIOP_PRIVATE_H

#include <jive/bitstring/multiop.h>
#include <jive/bitstring/type.h>
#include <jive/vsdg/normalization-private.h>

void
_jive_bitstring_multiop_node_init(
	jive_node * self,
	struct jive_region * region,
	size_t noperands,
	struct jive_output * operands[const],
	size_t nbits);

bool
_jive_bitstring_multiop_node_equiv(const jive_node_attrs * first, const jive_node_attrs * second);

bool
_jive_bitstring_multiop_node_can_reduce(const jive_output * first, const jive_output * second);

void
_jive_bitstring_keepwidth_multiop_node_init(
	jive_node * self,
	struct jive_region * region,
	size_t noperands,
	struct jive_output * operands[const]);

void
_jive_bitstring_expandwidth_multiop_node_init(
	jive_node * self,
	struct jive_region * region,
	size_t noperands,
	struct jive_output * operands[const]);

#define MAKE_KEEPWIDTH_OP(instancename, CLASSNAME) \
 \
static char * \
_jive_##instancename##_node_get_label(const jive_node * self) \
{ \
	return strdup(#CLASSNAME); \
} \
 \
static jive_node * \
_jive_##instancename##_node_create(struct jive_region * region, const jive_node_attrs * attrs, \
	size_t noperands, struct jive_output * operands[]) \
{ \
	return jive_##instancename##_node_create(region, noperands, operands); \
} \
 \
jive_output * \
_jive_##instancename##_node_reduce(jive_output * first_, jive_output * second_) \
{ \
	if ((first_->node->class_ != &JIVE_BITCONSTANT_NODE) || (second_->node->class_ != &JIVE_BITCONSTANT_NODE)) \
		return 0; \
	 \
	const jive_bitconstant_node * first = (const jive_bitconstant_node *) first_->node; \
	const jive_bitconstant_node * second = (const jive_bitconstant_node *) second_->node; \
	 \
	size_t nbits = first->attrs.nbits; \
	char bits[nbits]; \
	_jive_##instancename##_node_constant_reduce(bits, \
		first->attrs.bits, first->attrs.nbits, \
		second->attrs.bits, second->attrs.nbits); \
	 \
	return jive_bitconstant_node_create(first->base.graph, nbits, bits)->base.outputs[0]; \
} \
 \
const jive_node_class JIVE_##CLASSNAME##_NODE = { \
	.parent = &JIVE_BITSTRING_KEEPWIDTH_MULTIOP_NODE, \
	.flags = jive_node_class_associative | jive_node_class_commutative, \
	.fini = _jive_node_fini, /* inherit */ \
	.get_label = _jive_##instancename##_node_get_label, /* override */ \
	.get_attrs = _jive_node_get_attrs, /* inherit */ \
	.create = _jive_##instancename##_node_create, /* override */ \
	.equiv = _jive_bitstring_multiop_node_equiv, /* inherit */ \
	.can_reduce = _jive_bitstring_multiop_node_can_reduce, /* inherit */ \
	.reduce = _jive_##instancename##_node_reduce, /* inherit */ \
	.get_aux_regcls = _jive_node_get_aux_regcls /* inherit */ \
}; \
 \
jive_##instancename##_node * \
jive_##instancename##_node_create( \
	jive_region * region, \
	size_t noperands, jive_output * operands[const]) \
{ \
	jive_##instancename##_node * node = jive_context_malloc(region->graph->context, sizeof(*node)); \
	_jive_bitstring_keepwidth_multiop_node_init(node, region, noperands, operands); \
	return node; \
} \

#define MAKE_EXPANDWIDTH_OP(instancename, CLASSNAME, node_flags) \
 \
static char * \
_jive_##instancename##_node_get_label(const jive_node * self) \
{ \
	return strdup(#CLASSNAME); \
} \
 \
static jive_node * \
_jive_##instancename##_node_create(struct jive_region * region, const jive_node_attrs * attrs, \
	size_t noperands, struct jive_output * operands[]) \
{ \
	return jive_##instancename##_node_create(region, noperands, operands); \
} \
 \
jive_output * \
_jive_##instancename##_node_reduce(jive_output * first_, jive_output * second_) \
{ \
	if ((first_->node->class_ != &JIVE_BITCONSTANT_NODE) || (second_->node->class_ != &JIVE_BITCONSTANT_NODE)) \
		return 0; \
	 \
	const jive_bitconstant_node * first = (const jive_bitconstant_node *) first_->node; \
	const jive_bitconstant_node * second = (const jive_bitconstant_node *) second_->node; \
	 \
	size_t nbits = first->attrs.nbits + second->attrs.nbits; \
	char bits[nbits]; \
	_jive_##instancename##_node_constant_reduce(bits, \
		first->attrs.bits, first->attrs.nbits, \
		second->attrs.bits, second->attrs.nbits); \
	 \
	return jive_bitconstant_node_create(first->base.graph, nbits, bits)->base.outputs[0]; \
} \
 \
const jive_node_class JIVE_##CLASSNAME##_NODE = { \
	.parent = &JIVE_BITSTRING_EXPANDWIDTH_MULTIOP_NODE, \
	.flags = node_flags, \
	.fini = _jive_node_fini, /* inherit */ \
	.get_label = _jive_##instancename##_node_get_label, /* override */ \
	.get_attrs = _jive_node_get_attrs, /* inherit */ \
	.create = _jive_##instancename##_node_create, /* override */ \
	.equiv = _jive_bitstring_multiop_node_equiv, /* inherit */ \
	.can_reduce = _jive_bitstring_multiop_node_can_reduce, /* inherit */ \
	.reduce = _jive_##instancename##_node_reduce, /* inherit */ \
	.get_aux_regcls = _jive_node_get_aux_regcls /* inherit */ \
}; \
 \
jive_##instancename##_node * \
jive_##instancename##_node_create( \
	jive_region * region, \
	size_t noperands, jive_output * operands[const]) \
{ \
	jive_##instancename##_node * node = jive_context_malloc(region->graph->context, sizeof(*node)); \
	_jive_bitstring_expandwidth_multiop_node_init(node, region, noperands, operands); \
	return node; \
} \

#define MAKE_OP_HELPER(opname, CLASSNAME) \
 \
jive_bitstring * \
jive_##opname( \
	size_t noperands, jive_bitstring * operands_[const]) \
{ \
	jive_node_attrs attrs = {}; \
	jive_output * operands[noperands]; \
	size_t n; \
	for(n=0; n<noperands; n++) operands[n] = &operands_[n]->base.base; \
	jive_node * node =jive_node_normalized_create(&JIVE_##CLASSNAME##_NODE, &attrs, noperands, operands); \
	return (jive_bitstring *) node->outputs[0]; \
} \

#endif
