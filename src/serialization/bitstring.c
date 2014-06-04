/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/serialization/driver.h>
#include <jive/serialization/grammar.h>
#include <jive/serialization/nodecls-registry.h>
#include <jive/serialization/token-stream.h>
#include <jive/serialization/typecls-registry.h>

#include <jive/types/bitstring.h>

static void
jive_bitconstant_serialize(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	const jive_node_attrs * attrs_, jive_token_ostream * os)
{
	const jive::bitstring::constant_operation * attrs =
		(const jive::bitstring::constant_operation *) attrs_;
	jive_token_ostream_string(os, &attrs->bits[0], attrs->bits.size());
}

static bool
jive_bitconstant_deserialize(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	jive_region * region, size_t noperands,
	jive_output * const operands[], jive_token_istream * is,
	jive_node ** node)
{
	const jive_token * token = jive_token_istream_current(is);
	if (token->type != jive_token_string) {
		driver->error(driver, "Expected string");
		return false;
	}
	jive::bitstring::constant_operation attrs;
	
	char * bits = token->v.string.str;
	size_t nbits = token->v.string.len;
	attrs.bits = std::vector<char>(bits, bits + nbits);
	
	*node = JIVE_BITCONSTANT_NODE.create(region, &attrs,
		noperands, operands);
	
	jive_token_istream_advance(is);
	
	return true;
}

JIVE_SERIALIZATION_NODECLS_REGISTER(
	JIVE_BITCONSTANT_NODE, "bitconstant",
	jive_bitconstant_serialize,
	jive_bitconstant_deserialize);

static void
jive_bitslice_serialize(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	const jive_node_attrs * attrs_, jive_token_ostream * os)
{
	const jive::bitstring::slice_operation * attrs = (const jive::bitstring::slice_operation *) attrs_;
	jive_token_ostream_integral(os, attrs->low());
	jive_token_ostream_char(os, ',');
	jive_token_ostream_integral(os, attrs->high());
}

static bool
jive_bitslice_deserialize(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	jive_region * region, size_t noperands,
	jive_output * const operands[], jive_token_istream * is,
	jive_node ** node)
{
	uint64_t low, high;
	if (!jive_deserialize_uint(driver, is, &low))
		return false;
	if (!jive_deserialize_char_token(driver, is, ','))
		return false;
	if (!jive_deserialize_uint(driver, is, &high))
		return false;
	/* FIXME: check low < high, high < nbits */

	jive::bitstring::slice_operation attrs(low, high);
	
	*node = JIVE_BITSLICE_NODE.create(region, &attrs,
		noperands, operands);
	
	return true;
}

template<typename Operation>
static bool
jive_serialization_nodecls_deserialize_bitbinary1(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	jive_region * region,
	size_t noperands, jive_output * const operands[],
	struct jive_token_istream * is,
	jive_node ** node)
{
	Operation op(dynamic_cast<const jive_bitstring_type&>(operands[0]->type()), noperands);
	*node = jive_node_create(self->cls, op, region, noperands, operands);
	return *node != 0;
}
#define JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY1(nodecls, opcls, tag) \
	static void __attribute__((constructor)) register_##nodecls(void)\
	{ \
		jive_serialization_nodecls_register(&nodecls, tag, \
			jive_serialization_nodecls_serialize_default, \
			jive_serialization_nodecls_deserialize_bitbinary1<opcls>); \
	} \

template<typename Operation>
static bool
jive_serialization_nodecls_deserialize_bitbinary2(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	jive_region * region,
	size_t noperands, jive_output * const operands[],
	struct jive_token_istream * is,
	jive_node ** node)
{
	Operation op(dynamic_cast<const jive_bitstring_type&>(operands[0]->type()));
	*node = jive_node_create(self->cls, op, region, noperands, operands);
	return *node != 0;
}
#define JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY2(nodecls, opcls, tag) \
	static void __attribute__((constructor)) register_##nodecls(void)\
	{ \
		jive_serialization_nodecls_register(&nodecls, tag, \
			jive_serialization_nodecls_serialize_default, \
			jive_serialization_nodecls_deserialize_bitbinary2<opcls>); \
	} \

template<typename Operation>
static bool
jive_serialization_nodecls_deserialize_bitunary(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	jive_region * region,
	size_t noperands, jive_output * const operands[],
	struct jive_token_istream * is,
	jive_node ** node)
{
	Operation op(dynamic_cast<const jive_bitstring_type&>(operands[0]->type()));
	*node = jive_node_create(self->cls, op, region, noperands, operands);
	return *node != 0;
}
#define JIVE_SERIALIZATION_NODECLS_REGISTER_BITUNARY(nodecls, opcls, tag) \
	static void __attribute__((constructor)) register_##nodecls(void)\
	{ \
		jive_serialization_nodecls_register(&nodecls, tag, \
			jive_serialization_nodecls_serialize_default, \
			jive_serialization_nodecls_deserialize_bitunary<opcls>); \
	} \

JIVE_SERIALIZATION_NODECLS_REGISTER(
	JIVE_BITSLICE_NODE, "bitslice",
	jive_bitslice_serialize,
	jive_bitslice_deserialize);

JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITCONCAT_NODE, "bitconcat");

JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY1(
	JIVE_BITAND_NODE, jive::bitstring::and_operation, "bitand");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY1(
	JIVE_BITOR_NODE, jive::bitstring::or_operation, "bitor");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY1(
	JIVE_BITXOR_NODE, jive::bitstring::xor_operation, "bitxor");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITUNARY(
	JIVE_BITNOT_NODE, jive::bitstring::not_operation, "bitnot");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITUNARY(
	JIVE_BITNEGATE_NODE, jive::bitstring::negate_operation, "bitnegate");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY2(
	JIVE_BITSHL_NODE, jive::bitstring::shl_operation, "bitshl");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY2(
	JIVE_BITSHR_NODE, jive::bitstring::shr_operation, "bitshr");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY2(
	JIVE_BITASHR_NODE, jive::bitstring::ashr_operation, "bitashr");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY1(
	JIVE_BITSUM_NODE, jive::bitstring::sum_operation, "bitsum");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY2(
	JIVE_BITDIFFERENCE_NODE, jive::bitstring::difference_operation, "bitdifference");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY1(
	JIVE_BITPRODUCT_NODE, jive::bitstring::product_operation, "bitproduct");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY2(
	JIVE_BITSHIPRODUCT_NODE, jive::bitstring::shiproduct_operation, "bitshiproduct");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY2(
	JIVE_BITUHIPRODUCT_NODE, jive::bitstring::uhiproduct_operation, "bituhiproduct");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY2(
	JIVE_BITSQUOTIENT_NODE, jive::bitstring::squotient_operation, "bitsquotient");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY2(
	JIVE_BITUQUOTIENT_NODE, jive::bitstring::uquotient_operation, "bituquotient");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY2(
	JIVE_BITSMOD_NODE, jive::bitstring::smod_operation, "bitsmod");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY2(
	JIVE_BITUMOD_NODE, jive::bitstring::umod_operation, "bitumod");

JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITEQUAL_NODE, "bitequal");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITNOTEQUAL_NODE, "bitnotequal");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITSGREATEREQ_NODE, "bitsgreatereq");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITSGREATER_NODE, "bitsgreater");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITSLESSEQ_NODE, "bitslesseq");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITSLESS_NODE, "bitsless");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITUGREATEREQ_NODE, "bitugreatereq");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITUGREATER_NODE, "bitugreater");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITULESSEQ_NODE, "bitulesseq");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITULESS_NODE, "bituless");

static void
jive_bitstring_type_serialize(
	const jive_serialization_typecls * self,
	jive_serialization_driver * driver,
	const jive_type * type_,
	jive_token_ostream * os)
{
	const jive_bitstring_type * type = (const jive_bitstring_type *) type_;
	jive_serialize_uint(driver, type->nbits(), os);
}

bool
jive_bitstring_type_deserialize(
	const jive_serialization_typecls * self,
	jive_serialization_driver * driver,
	jive_token_istream * is,
	jive_type ** type)
{
	uint64_t nbits;
	if (!jive_deserialize_uint(driver, is, &nbits))
		return false;
	
	/* FIXME: check number of bits */
	
	*type = new jive_bitstring_type(nbits);
	return true;
}

JIVE_SERIALIZATION_TYPECLS_REGISTER(
	jive_bitstring_type, jive_bitstring_type, "bits",
	jive_bitstring_type_serialize,
	jive_bitstring_type_deserialize);
