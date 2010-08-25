#include <jive/bitstring/multiop-private.h>
#include <jive/bitstring/constant.h>
#include <jive/bitstring/bitops-private.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/graph.h>
#include <string.h>

const jive_node_class JIVE_BITSTRING_MULTIOP_NODE = {
	.parent = &JIVE_NODE,
	.fini = _jive_node_fini, /* inherit */
	.get_label = _jive_node_get_label, /* inherit */
	.get_attrs = _jive_node_get_attrs, /* inherit */
	.create = _jive_node_create, /* inherit */
	.equiv = _jive_bitstring_multiop_node_equiv, /* override */
	.can_reduce = _jive_bitstring_multiop_node_can_reduce, /* override */
	.reduce = _jive_node_reduce, /* inherit */
	.get_aux_regcls = _jive_node_get_aux_regcls /* inherit */
};

const jive_node_class JIVE_BITSTRING_KEEPWIDTH_MULTIOP_NODE = {
	.parent = &JIVE_BITSTRING_MULTIOP_NODE,
	.fini = _jive_node_fini, /* inherit */
	.get_label = _jive_node_get_label, /* inherit */
	.get_attrs = _jive_node_get_attrs, /* inherit */
	.create = _jive_node_create, /* inherit */
	.equiv = _jive_bitstring_multiop_node_equiv, /* inherit */
	.can_reduce = _jive_bitstring_multiop_node_can_reduce, /* inherit */
	.reduce = _jive_node_reduce, /* inherit */
	.get_aux_regcls = _jive_node_get_aux_regcls /* inherit */
};

const jive_node_class JIVE_BITSTRING_EXPANDWIDTH_MULTIOP_NODE = {
	.parent = &JIVE_BITSTRING_MULTIOP_NODE,
	.fini = _jive_node_fini, /* inherit */
	.get_label = _jive_node_get_label, /* inherit */
	.get_attrs = _jive_node_get_attrs, /* inherit */
	.create = _jive_node_create, /* inherit */
	.equiv = _jive_bitstring_multiop_node_equiv, /* inherit */
	.can_reduce = _jive_bitstring_multiop_node_can_reduce, /* inherit */
	.reduce = _jive_node_reduce, /* inherit */
	.get_aux_regcls = _jive_node_get_aux_regcls /* inherit */
};

void
_jive_bitstring_multiop_node_init(
	jive_node * self,
	struct jive_region * region,
	size_t noperands,
	struct jive_output * operands[const],
	size_t nbits)
{
	const jive_type * operand_types[noperands];
	jive_bitstring_type operand_type_structs[noperands];
	size_t n;
	
	for(n=0; n<noperands; n++) {
		operand_type_structs[n].base.base.class_ = &JIVE_BITSTRING_TYPE;
		operand_type_structs[n].nbits = ((const jive_bitstring_type *)jive_output_get_type(operands[n]))->nbits;
		operand_types[n] = &operand_type_structs[n].base.base;
	}
	
	JIVE_DECLARE_BITSTRING_TYPE(output_type, nbits);
	
	_jive_node_init(self, region,
		noperands, operand_types, operands,
		1, &output_type);
}

bool
_jive_bitstring_multiop_node_equiv(const jive_node_attrs * first, const jive_node_attrs * second)
{
	return true;
}

bool
_jive_bitstring_multiop_node_can_reduce(const jive_output * first, const jive_output * second)
{
	return (first->node->class_ == &JIVE_BITCONSTANT_NODE) && (second->node->class_ == &JIVE_BITCONSTANT_NODE);
}

void
_jive_bitstring_keepwidth_multiop_node_init(
	jive_node * self,
	struct jive_region * region,
	size_t noperands,
	struct jive_output * operands[const])
{
	size_t nbits = ((const jive_bitstring_type *)jive_output_get_type(operands[0]))->nbits, n;
	for(n=0; n<noperands; n++)
		if (((const jive_bitstring_type *)jive_output_get_type(operands[n]))->nbits != nbits)
			jive_context_fatal_error(region->graph->context, "All operands must have same width");
	_jive_bitstring_multiop_node_init(self, region, noperands, operands, nbits);
}

void
_jive_bitstring_expandwidth_multiop_node_init(
	jive_node * self,
	struct jive_region * region,
	size_t noperands,
	struct jive_output * operands[const])
{
	size_t nbits = 0, n;
	for(n=0; n<noperands; n++)
		nbits += ((const jive_bitstring_type *)jive_output_get_type(operands[n]))->nbits;
	_jive_bitstring_multiop_node_init(self, region, noperands, operands, nbits);
}

/* bit-wise and */

static char *
_jive_bitand_node_get_label(const jive_node * self)
{
	return strdup("BITAND");
}

static jive_node *
_jive_bitand_node_create(const jive_node_attrs * attrs, struct jive_region * region,
	size_t noperands, struct jive_output * operands[])
{
	return jive_bitand_node_create(region, noperands, operands);
}

jive_output *
_jive_bitand_node_reduce(jive_output * first_, jive_output * second_)
{
	if ((first_->node->class_ != &JIVE_BITCONSTANT_NODE) || (second_->node->class_ != &JIVE_BITCONSTANT_NODE))
		return 0;
	
	const jive_bitconstant_node * first = (const jive_bitconstant_node *) first_->node;
	const jive_bitconstant_node * second = (const jive_bitconstant_node *) second_->node;
	
	size_t n, nbits = first->attrs.nbits;
	char bits[nbits];
	for(n=0; n<nbits; n++)
		bits[n] = jive_logic_and(first->attrs.bits[n], second->attrs.bits[n]);
	
	return jive_bitconstant_node_create(first->base.graph, nbits, bits)->base.outputs[0];
}

const jive_node_class JIVE_BITAND_NODE = {
	.parent = &JIVE_BITSTRING_KEEPWIDTH_MULTIOP_NODE,
	.fini = _jive_node_fini, /* inherit */
	.get_label = _jive_bitand_node_get_label, /* override */
	.get_attrs = _jive_node_get_attrs, /* inherit */
	.create = _jive_bitand_node_create, /* override */
	.equiv = _jive_bitstring_multiop_node_equiv, /* inherit */
	.can_reduce = _jive_bitstring_multiop_node_can_reduce, /* inherit */
	.reduce = _jive_bitand_node_reduce, /* inherit */
	.get_aux_regcls = _jive_node_get_aux_regcls /* inherit */
};

jive_bitand_node *
jive_bitand_node_create(
	jive_region * region,
	size_t noperands, jive_output * operands[const])
{
	jive_bitand_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	_jive_bitstring_keepwidth_multiop_node_init(node, region, noperands, operands);
	return node;
}
