#include <jive/bitstring/slice.h>
#include <jive/bitstring/type.h>
#include <jive/bitstring/constant.h>
#include <jive/bitstring/symbolic-constant.h>
#include <jive/bitstring/multiop.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg.h>
#include <string.h>
#include <stdio.h>
#include <jive/vsdg/normalization-private.h>

static void
_jive_bitslice_node_init(
	jive_bitslice_node * self,
	jive_region * region,
	jive_output * origin,
	size_t low, size_t high);

static char *
_jive_bitslice_node_get_label(const jive_node * self);

static const jive_node_attrs *
_jive_bitslice_node_get_attrs(const jive_node * self);

static jive_node *
_jive_bitslice_node_create(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * operands[]);

static bool
_jive_bitslice_node_equiv(const jive_node_attrs * first, const jive_node_attrs * second);

const jive_node_class JIVE_BITSLICE_NODE = {
	.parent = &JIVE_NODE,
	.fini = _jive_node_fini, /* inherit */
	.get_label = _jive_bitslice_node_get_label, /* override */
	.get_attrs = _jive_bitslice_node_get_attrs, /* override */
	.create = _jive_bitslice_node_create, /* override */
	.equiv = _jive_bitslice_node_equiv, /* override */
	.can_reduce = _jive_node_can_reduce, /* inherit */
	.reduce = _jive_node_reduce, /* inherit */
	.get_aux_regcls = _jive_node_get_aux_regcls /* inherit */
};

static void
_jive_bitslice_node_init(
	jive_bitslice_node * self,
	jive_region * region,
	jive_output * origin,
	size_t low, size_t high)
{
	JIVE_DECLARE_BITSTRING_TYPE(input_type, ((jive_bitstring_output *) origin)->type.nbits);
	JIVE_DECLARE_BITSTRING_TYPE(output_type, high - low);
	_jive_node_init(&self->base, region,
		1, &input_type, &origin,
		1, &output_type);
	self->attrs.low = low;
	self->attrs.high = high;
}

static char *
_jive_bitslice_node_get_label(const jive_node * self_)
{
	const jive_bitslice_node * self = (const jive_bitslice_node *) self_;
	
	char tmp[32];
	snprintf(tmp, sizeof(tmp), "SLICE[%zd:%zd)", self->attrs.low, self->attrs.high);
	return strdup(tmp);
}

static const jive_node_attrs *
_jive_bitslice_node_get_attrs(const jive_node * self_)
{
	const jive_bitslice_node * self = (const jive_bitslice_node *) self_;
	return &self->attrs.base;
}

static jive_node *
_jive_bitslice_node_create(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * operands[])
{
	const jive_bitslice_node_attrs * attrs = (const jive_bitslice_node_attrs *) attrs_;
	return &jive_bitslice_node_create(region, operands[0], attrs->low, attrs->high)->base;
}

static bool
_jive_bitslice_node_equiv(const jive_node_attrs * first_, const jive_node_attrs * second_)
{
	const jive_bitslice_node_attrs * first = (const jive_bitslice_node_attrs *) first_;
	const jive_bitslice_node_attrs * second = (const jive_bitslice_node_attrs *) second_;
	return (first->low == second->low) && (first->high == second->high);
}

jive_bitslice_node *
jive_bitslice_node_create(struct jive_region * region, jive_output * origin, size_t low, size_t high)
{
	jive_bitslice_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->base.class_ = &JIVE_BITSLICE_NODE;
	_jive_bitslice_node_init(node, region, origin, low, high);
	return node;
}

jive_bitstring *
jive_bitslice(jive_bitstring * operand, size_t low, size_t high)
{
	if ((low == 0) && (high == operand->type.nbits)) return operand; 
	jive_output * origin = &operand->base.base;
	
	if (origin->node->class_ == &JIVE_BITSLICE_NODE) {
		const jive_bitslice_node * node = (const jive_bitslice_node *) operand->base.base.node;
		operand = (jive_bitstring *) node->base.inputs[0];
		return jive_bitslice(operand, low + node->attrs.low, high + node->attrs.low);
	}
	
	if (origin->node->class_ == &JIVE_BITCONSTANT_NODE) {
		const jive_bitconstant_node * node = (const jive_bitconstant_node *) operand->base.base.node;
		return jive_bitconstant_create(node->base.graph, high - low, node->attrs.bits + low);
	}
	
	if (origin->node->class_ == &JIVE_BITCONCAT_NODE) {
		jive_node * node = origin->node;
		jive_bitstring * operands[node->ninputs];
		size_t noperands = 0, pos = 0, n;
		for(n=0; n<node->noperands; n++) {
			jive_bitstring * operand = (jive_bitstring *) node->inputs[n]->origin;
			size_t base = pos;
			pos = pos + operand->type.nbits;
			if (base < high && pos > low) {
				size_t slice_low = (low > base) ? (low - base) : 0;
				size_t slice_high = (high < pos) ? (high - base) : (pos-base);
				operand = jive_bitslice(operand, slice_low, slice_high);
				operands[noperands++] = operand;
			}
		}
		
		return jive_bitconcat(noperands, operands);
	}
	
	jive_bitslice_node_attrs attrs;
	attrs.low = low;
	attrs.high = high;
	jive_node * node = jive_node_cse(&JIVE_BITSLICE_NODE, operand->base.base.node->graph, &attrs.base, 1, &origin);
	if (!node) node = (jive_node *) jive_bitslice_node_create(origin->node->region, origin, low, high);
	return (jive_bitstring_output *) node->outputs[0];
}
