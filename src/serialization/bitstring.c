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
	const jive_bitconstant_node_attrs * attrs = (const jive_bitconstant_node_attrs *) attrs_;
	jive_token_ostream_string(os, attrs->bits, attrs->nbits);
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
	jive_bitconstant_node_attrs attrs;
	attrs.bits = (char *) token->v.string.str;
	attrs.nbits = token->v.string.len;
	
	*node = JIVE_BITCONSTANT_NODE.create(region, &attrs.base,
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
	const jive_bitslice_node_attrs * attrs = (const jive_bitslice_node_attrs *) attrs_;
	jive_token_ostream_integral(os, attrs->low);
	jive_token_ostream_char(os, ',');
	jive_token_ostream_integral(os, attrs->high);
}

static bool
jive_bitslice_deserialize(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	jive_region * region, size_t noperands,
	jive_output * const operands[], jive_token_istream * is,
	jive_node ** node)
{
	jive_bitslice_node_attrs attrs;
	
	uint64_t low, high;
	if (!jive_deserialize_uint(driver, is, &low))
		return false;
	if (!jive_deserialize_char_token(driver, is, ','))
		return false;
	if (!jive_deserialize_uint(driver, is, &high))
		return false;
	/* FIXME: check low < high, high < nbits */
	
	attrs.low = low;
	attrs.high = high;
	
	*node = JIVE_BITSLICE_NODE.create(region, &attrs.base,
		noperands, operands);
	
	return true;
}

JIVE_SERIALIZATION_NODECLS_REGISTER(
	JIVE_BITSLICE_NODE, "bitslice",
	jive_bitslice_serialize,
	jive_bitslice_deserialize);

JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITCONCAT_NODE, "bitconcat");

JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITAND_NODE, "bitand");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITOR_NODE, "bitor");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITXOR_NODE, "bitxor");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITNOT_NODE, "bitnot");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITNEGATE_NODE, "bitnegate");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITSHL_NODE, "bitshl");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITSHR_NODE, "bitshr");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITASHR_NODE, "bitashr");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITSUM_NODE, "bitsum");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITDIFFERENCE_NODE, "bitdifference");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITPRODUCT_NODE, "bitproduct");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITSHIPRODUCT_NODE, "bitshiproduct");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITUHIPRODUCT_NODE, "bituhiproduct");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITSQUOTIENT_NODE, "bitsquotient");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITUQUOTIENT_NODE, "bituquotient");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITSMOD_NODE, "bitsmod");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_BITUMOD_NODE, "bitumod");

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
	jive_serialize_uint(driver, type->nbits, os);
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
	
	JIVE_DECLARE_BITSTRING_TYPE(ctl, nbits);
	*type = jive_type_copy(ctl, driver->context);
	return true;
}

JIVE_SERIALIZATION_TYPECLS_REGISTER(JIVE_BITSTRING_TYPE, "bits", jive_bitstring_type_serialize, jive_bitstring_type_deserialize);
