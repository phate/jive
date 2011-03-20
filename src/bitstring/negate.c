#include <jive/bitstring/negate.h>

#include <stdio.h>
#include <string.h>

#include <jive/common.h>

#include <jive/bitstring/bitops-private.h>
#include <jive/bitstring/concat.h>
#include <jive/bitstring/constant.h>
#include <jive/bitstring/type.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/operators.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

static void
_jive_bitnegate_node_init(
	jive_node * self,
	jive_region * region,
	jive_output * origin);

static char *
_jive_bitnegate_node_get_label(const jive_node * self);

static jive_node *
_jive_bitnegate_node_create(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

static bool
jive_bitnegate_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * operand);

static bool
jive_bitnegate_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output ** operand);

const jive_unary_operation_class JIVE_BITNEGATE_NODE_ = {
	.base = { /* jive_node_class */
		.parent = &JIVE_UNARY_OPERATION,
		.fini = _jive_node_fini, /* inherit */
		.get_label = _jive_bitnegate_node_get_label, /* override */
		.get_attrs = _jive_node_get_attrs, /* inherit */
		.match_attrs = _jive_node_match_attrs, /* inherit */
		.create = _jive_bitnegate_node_create, /* override */
		.get_aux_rescls = _jive_node_get_aux_rescls /* inherit */
	},
	
	.single_apply_over = NULL,
	.multi_apply_over = NULL,
	
	.can_reduce_operand = jive_bitnegate_can_reduce_operand_, /* override */
	.reduce_operand = jive_bitnegate_reduce_operand_ /* override */
};

static void
_jive_bitnegate_node_init(
	jive_node * self,
	jive_region * region,
	jive_output * origin)
{
	size_t nbits = ((jive_bitstring_output *) origin)->type.nbits;
	JIVE_DECLARE_BITSTRING_TYPE(type, nbits);
	_jive_node_init(self, region,
		1, &type, &origin,
		1, &type);
}

static char *
_jive_bitnegate_node_get_label(const jive_node * self_)
{
	return strdup("BITNEGATE");
}

static jive_node *
_jive_bitnegate_node_create(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);
	
	jive_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->class_ = &JIVE_BITNEGATE_NODE;
	_jive_bitnegate_node_init(node, region, operands[0]);
	return node;
}

static bool
jive_bitnegate_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs_, jive_output * operand_)
{
	if (operand_->node->class_ == &JIVE_BITNEGATE_NODE) return true;
	if (operand_->node->class_ == &JIVE_BITCONSTANT_NODE) return true;
	
	return false;
}

static bool
jive_bitnegate_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs_, jive_output ** operand_)
{
	jive_bitstring_output * operand = (jive_bitstring_output *) *operand_;
	
	if (operand->base.base.node->class_ == &JIVE_BITNEGATE_NODE) {
		jive_node * node = operand->base.base.node;
		*operand_ = node->inputs[0]->origin;
		return true;
	}
	
	if (operand->base.base.node->class_ == &JIVE_BITCONSTANT_NODE) {
		const jive_bitconstant_node * node = (const jive_bitconstant_node *) operand->base.base.node;
		char bits[node->attrs.nbits];
		
		jive_multibit_negate(bits, node->attrs.bits, node->attrs.nbits);
		
		*operand_ = jive_bitconstant(node->base.graph, node->attrs.nbits, bits);
		return true;
	}
	
	return false;
}

jive_node *
jive_bitnegate_create(struct jive_region * region, jive_output * operand)
{
	return jive_unary_operation_normalized_create(&JIVE_BITNEGATE_NODE, region, NULL, operand);
}

jive_output *
jive_bitnegate(jive_output * operand)
{
	return jive_bitnegate_create(operand->node->region, operand)->outputs[0];
}
