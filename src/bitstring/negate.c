#include <jive/bitstring/negate.h>
#include <jive/bitstring/type.h>
#include <jive/bitstring/constant.h>
#include <jive/bitstring/symbolic-constant.h>
#include <jive/bitstring/multiop.h>
#include <jive/bitstring/bitops-private.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg.h>
#include <string.h>
#include <stdio.h>
#include <jive/vsdg/normalization-private.h>

static void
_jive_bitnegate_node_init(
	jive_bitnegate_node * self,
	jive_region * region,
	jive_output * origin);

static char *
_jive_bitnegate_node_get_label(const jive_node * self);

static jive_node *
_jive_bitnegate_node_create(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * operands[]);

const jive_node_class JIVE_BITNEGATE_NODE = {
	.parent = &JIVE_NODE,
	.fini = _jive_node_fini, /* inherit */
	.get_label = _jive_bitnegate_node_get_label, /* override */
	.get_attrs = _jive_node_get_attrs, /* inherit */
	.create = _jive_bitnegate_node_create, /* override */
	.equiv = _jive_node_equiv, /* inherit */
	.can_reduce = _jive_node_can_reduce, /* inherit */
	.reduce = _jive_node_reduce, /* inherit */
	.get_aux_regcls = _jive_node_get_aux_regcls /* inherit */
};

static void
_jive_bitnegate_node_init(
	jive_bitnegate_node * self,
	jive_region * region,
	jive_output * origin)
{
	JIVE_DECLARE_BITSTRING_TYPE(type, ((jive_bitstring_output *) origin)->type.nbits);
	_jive_node_init(self, region,
		1, &type, &origin,
		1, &type);
}

static char *
_jive_bitnegate_node_get_label(const jive_node * self_)
{
	return strdup("NEGATE");
}

static jive_node *
_jive_bitnegate_node_create(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * operands[])
{
	return jive_bitnegate_node_create(region, operands[0]);
}

jive_bitnegate_node *
jive_bitnegate_node_create(struct jive_region * region, jive_output * origin)
{
	jive_bitnegate_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	_jive_bitnegate_node_init(node, region, origin);
	node->class_ = &JIVE_BITNEGATE_NODE;
	return node;
}

jive_bitstring *
jive_bitnegate(jive_bitstring * operand)
{
	jive_output * origin = &operand->base.base;
	
	if (origin->node->class_ == &JIVE_BITNEGATE_NODE) {
		return (jive_bitstring *) origin->node->inputs[0]->origin;
	}
	
	if (origin->node->class_ == &JIVE_BITCONSTANT_NODE) {
		const jive_bitconstant_node * node = (const jive_bitconstant_node *) operand->base.base.node;
		size_t nbits = node->attrs.nbits, n;
		char bits[nbits], carry = '1';
		for(n=0; n<nbits; n++) {
			char inv = jive_logic_xor(node->attrs.bits[n], '1');
			bits[n] = jive_logic_xor(inv, carry);
			carry = jive_logic_and(inv, carry);
		}
		return jive_bitconstant_create(node->base.graph, nbits, bits);
	}
	
	/* TODO: normalize wrt adds */
	
	jive_node * node = jive_node_cse(&JIVE_BITNEGATE_NODE, operand->base.base.node->graph, 0, 1, &origin);
	if (!node) node = (jive_node *) jive_bitnegate_node_create(origin->node->region, origin);
	return (jive_bitstring_output *) node->outputs[0];
}
