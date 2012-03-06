#include <jive/types/bitstring/slice.h>

#include <stdio.h>
#include <string.h>

#include <jive/common.h>

#include <jive/types/bitstring/arithmetic.h>
#include <jive/types/bitstring/concat.h>
#include <jive/types/bitstring/constant.h>
#include <jive/types/bitstring/type.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/operators.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

static void
jive_bitslice_node_init_(
	jive_bitslice_node * self,
	jive_region * region,
	jive_output * origin,
	size_t low, size_t high);

static char *
jive_bitslice_node_get_label_(const jive_node * self);

static const jive_node_attrs *
jive_bitslice_node_get_attrs_(const jive_node * self);

static bool
jive_bitslice_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs);

static jive_node *
jive_bitslice_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

static jive_unop_reduction_path_t
jive_bitslice_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs,
	const jive_output * operand);

static jive_output *
jive_bitslice_reduce_operand_(jive_unop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs, jive_output * operand);

const jive_unary_operation_class JIVE_BITSLICE_NODE_ = {
	.base = { /* jive_node_class */
		/* note that parent is JIVE_UNARY_OPERATION, not
		JIVE_BITUNARY_OPERATION: the latter one is assumed
		to represent "width-preserving" bit operations (i.e.
		number of bits per operand/output matches), while
		the slice operator violates this assumption */
		.parent = &JIVE_UNARY_OPERATION,
		.fini = jive_node_fini_, /* inherit */
		.get_default_normal_form = jive_unary_operation_get_default_normal_form_, /* inherit */
		.get_label = jive_bitslice_node_get_label_, /* override */
		.get_attrs = jive_bitslice_node_get_attrs_, /* override */
		.match_attrs = jive_bitslice_node_match_attrs_, /* override */
		.create = jive_bitslice_node_create_, /* override */
		.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
	},
	
	.single_apply_over = NULL,
	.multi_apply_over = NULL,
	
	.can_reduce_operand = jive_bitslice_can_reduce_operand_, /* override */
	.reduce_operand = jive_bitslice_reduce_operand_ /* override */
};

static void
jive_bitslice_node_init_(
	jive_bitslice_node * self,
	jive_region * region,
	jive_output * origin,
	size_t low, size_t high)
{
	JIVE_DECLARE_BITSTRING_TYPE(input_type, ((jive_bitstring_output *) origin)->type.nbits);
	JIVE_DECLARE_BITSTRING_TYPE(output_type, high - low);
	jive_node_init_(&self->base, region,
		1, &input_type, &origin,
		1, &output_type);
	self->attrs.low = low;
	self->attrs.high = high;
}

static char *
jive_bitslice_node_get_label_(const jive_node * self_)
{
	const jive_bitslice_node * self = (const jive_bitslice_node *) self_;
	
	char tmp[32];
	snprintf(tmp, sizeof(tmp), "SLICE[%zd:%zd)", self->attrs.low, self->attrs.high);
	return strdup(tmp);
}

static const jive_node_attrs *
jive_bitslice_node_get_attrs_(const jive_node * self_)
{
	const jive_bitslice_node * self = (const jive_bitslice_node *) self_;
	return &self->attrs.base;
}

static bool
jive_bitslice_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_bitslice_node_attrs * first = &((const jive_bitslice_node *)self)->attrs;
	const jive_bitslice_node_attrs * second = (const jive_bitslice_node_attrs *) attrs;
	return (first->low == second->low) && (first->high == second->high);
}

static jive_node *
jive_bitslice_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);
	
	const jive_bitslice_node_attrs * attrs = (const jive_bitslice_node_attrs *) attrs_;
	
	jive_bitslice_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->base.class_ = &JIVE_BITSLICE_NODE;
	jive_bitslice_node_init_(node, region, operands[0], attrs->low, attrs->high);
	return &node->base;
}

static jive_unop_reduction_path_t
jive_bitslice_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs_,
	const jive_output * operand_)
{
	const jive_bitslice_node_attrs * attrs = (const jive_bitslice_node_attrs *) attrs_;
	jive_bitstring_output * operand = (jive_bitstring_output *) operand_;
	
	if ((attrs->low == 0) && (attrs->high == operand->type.nbits))
		return jive_unop_reduction_idempotent;
	if (operand_->node->class_ == &JIVE_BITSLICE_NODE)
		return jive_unop_reduction_narrow;
	if (operand_->node->class_ == &JIVE_BITCONSTANT_NODE)
		return jive_unop_reduction_constant;
	if (operand_->node->class_ == &JIVE_BITCONCAT_NODE)
		return jive_unop_reduction_distribute;
	
	return jive_unop_reduction_none;
}

static jive_output *
jive_bitslice_reduce_operand_(jive_unop_reduction_path_t path,
	const jive_node_class * cls, const jive_node_attrs * attrs_, jive_output * operand_)
{
	const jive_bitslice_node_attrs * attrs = (const jive_bitslice_node_attrs *) attrs_;
	jive_bitstring_output * operand = (jive_bitstring_output *) operand_;
	
	if (path == jive_unop_reduction_idempotent)
		return operand_;
	
	if (path == jive_unop_reduction_narrow) {
		const jive_bitslice_node * node = (const jive_bitslice_node *) operand->base.base.node;
		return jive_bitslice(node->base.inputs[0]->origin, attrs->low + node->attrs.low,
			attrs->high + node->attrs.low);
	}
	
	if (path == jive_unop_reduction_constant) {
		const jive_bitconstant_node * node = (const jive_bitconstant_node *) operand->base.base.node;
		return jive_bitconstant(node->base.graph, attrs->high - attrs->low,
			node->attrs.bits + attrs->low);
	}
	
	if (path == jive_unop_reduction_distribute) {
		jive_node * node = operand->base.base.node;
		jive_output * operands[node->ninputs];
		
		size_t noperands = 0, pos = 0, n;
		for (n=0; n<node->noperands; n++) {
			jive_output * operand = node->inputs[n]->origin;
			size_t base = pos;
			size_t nbits = ((jive_bitstring_output *)operand)->type.nbits;
			pos = pos + nbits;
			if (base < attrs->high && pos > attrs->low) {
				size_t slice_low = (attrs->low > base) ? (attrs->low - base) : 0;
				size_t slice_high = (attrs->high < pos) ? (attrs->high - base) : (pos-base);
				operand = jive_bitslice(operand, slice_low, slice_high);
				operands[noperands++] = operand;
			}
		}
		
		return jive_bitconcat(noperands, operands);
	}
	
	return NULL;
}

jive_node *
jive_bitslice_create(struct jive_region * region, jive_output * operand, size_t low, size_t high)
{
	jive_bitslice_node_attrs attrs;
	attrs.low = low;
	attrs.high = high;
	
	const jive_unary_operation_normal_form * nf =
		(const jive_unary_operation_normal_form *)
		jive_graph_get_nodeclass_form(region->graph, &JIVE_BITSLICE_NODE);

	return jive_unary_operation_normalized_create(nf, region, &attrs.base, operand)->node;
}

jive_output *
jive_bitslice(jive_output * operand, size_t low, size_t high)
{
	jive_bitslice_node_attrs attrs;
	attrs.low = low;
	attrs.high = high;
	
	const jive_unary_operation_normal_form * nf =
		(const jive_unary_operation_normal_form *)
		jive_graph_get_nodeclass_form(operand->node->graph, &JIVE_BITSLICE_NODE);

	return jive_unary_operation_normalized_create(nf, operand->node->region, &attrs.base, operand);
}
