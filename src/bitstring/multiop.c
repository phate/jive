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

static void
_jive_bitand_node_constant_reduce(char * __restrict__ dst, const char * src1, size_t nbits1, const char * src2, size_t nbits2)
{
	size_t n;
	for(n=0; n<nbits1; n++) dst[n] = jive_logic_and(src1[n], src2[n]);
}

static void
_jive_bitor_node_constant_reduce(char * __restrict__ dst, const char * src1, size_t nbits1, const char * src2, size_t nbits2)
{
	size_t n;
	for(n=0; n<nbits1; n++) dst[n] = jive_logic_or(src1[n], src2[n]);
}

static void
_jive_bitxor_node_constant_reduce(char * __restrict__ dst, const char * src1, size_t nbits1, const char * src2, size_t nbits2)
{
	size_t n;
	for(n=0; n<nbits1; n++) dst[n] = jive_logic_xor(src1[n], src2[n]);
}

static void
_jive_bitsum_node_constant_reduce(char * __restrict__ dst, const char * src1, size_t nbits1, const char * src2, size_t nbits2)
{
	jive_multibit_sum(dst, src1, src2, nbits1);
}

static void
_jive_bitproduct_node_constant_reduce(char * __restrict__ dst, const char * src1, size_t nbits1, const char * src2, size_t nbits2)
{
	jive_multibit_multiply(dst, nbits1 + nbits2, src1, nbits1, src2, nbits2);
}

static void
_jive_bitconcat_node_constant_reduce(char * __restrict__ dst, const char * src1, size_t nbits1, const char * src2, size_t nbits2)
{
	size_t n;
	for(n=0; n<nbits1; n++) dst[n] = src1[n];
	for(n=0; n<nbits2; n++) dst[n+nbits1] = src2[n];
}

MAKE_KEEPWIDTH_OP(bitand, BITAND)
MAKE_KEEPWIDTH_OP(bitor, BITOR)
MAKE_KEEPWIDTH_OP(bitxor, BITXOR)
MAKE_KEEPWIDTH_OP(bitsum, BITSUM)
MAKE_EXPANDWIDTH_OP(bitproduct, BITPRODUCT)
MAKE_EXPANDWIDTH_OP(bitconcat, BITCONCAT)
