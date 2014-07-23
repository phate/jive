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
	const jive::bits::constant_op * attrs =
		(const jive::bits::constant_op *) attrs_;
	jive_token_ostream_string(os, &attrs->value()[0], attrs->value().size());
}

static bool
jive_bitconstant_deserialize(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	jive_region * region, size_t noperands,
	jive::output * const operands[], jive_token_istream * is,
	jive_node ** node)
{
	const jive_token * token = jive_token_istream_current(is);
	if (token->type != jive_token_string) {
		driver->error(driver, "Expected string");
		return false;
	}
	
	const char * bits = token->v.string.str;
	size_t nbits = token->v.string.len;

	jive::bits::constant_op op(jive::bits::value_repr(bits, bits + nbits));
	*node = op.create_node(region, noperands, operands);

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
	const jive::bits::slice_op * attrs = (const jive::bits::slice_op *) attrs_;
	jive_token_ostream_integral(os, attrs->low());
	jive_token_ostream_char(os, ',');
	jive_token_ostream_integral(os, attrs->high());
}

static bool
jive_bitslice_deserialize(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[],
	jive_token_istream * is,
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

	const jive::bits::type & argument_type =
		dynamic_cast<const jive::bits::type &>(arguments[0]->type());
	jive::bits::slice_op op(argument_type, low, high);

	*node = op.create_node(region, narguments, arguments);
	
	return true;
}

static void
jive_bitconcat_serialize(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	const jive_node_attrs * attrs_, jive_token_ostream * os)
{
	const jive::bits::concat_op * op = static_cast<const jive::bits::concat_op *>(attrs_);
	bool first = true;
	for (const jive::bits::type & t : op->argument_types()) {
		if (!first) {
			jive_token_ostream_char(os, ',');
		} else {
			first = false;
		}
		jive_token_ostream_integral(os, t.nbits());
	}
}

static bool
jive_bitconcat_deserialize(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[],
	jive_token_istream * is,
	jive_node ** node)
{
	std::vector<jive::bits::type> types;
	
	for(;;) {
		uint64_t bits;
		if (!jive_deserialize_uint(driver, is, &bits)) {
			return false;
		}
		types.emplace_back(bits);
		if (jive_token_istream_current(is)->type == jive_token_comma) {
			jive_token_istream_advance(is);
		} else {
			break;
		}
	}

	*node = jive::bits::concat_op(std::move(types)).create_node(region, narguments, arguments);
	
	return true;
}

template<typename Operation>
static bool
jive_serialization_nodecls_deserialize_bitbinary1(
	const jive_serialization_nodecls * self,
	struct jive_serialization_driver * driver,
	jive_region * region,
	size_t noperands, jive::output * const operands[],
	struct jive_token_istream * is,
	jive_node ** node)
{
	Operation op(dynamic_cast<const jive::bits::type&>(operands[0]->type()), noperands);
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
	size_t noperands, jive::output * const operands[],
	struct jive_token_istream * is,
	jive_node ** node)
{
	Operation op(dynamic_cast<const jive::bits::type&>(operands[0]->type()));
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
	size_t noperands, jive::output * const operands[],
	struct jive_token_istream * is,
	jive_node ** node)
{
	Operation op(dynamic_cast<const jive::bits::type&>(operands[0]->type()));
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

JIVE_SERIALIZATION_NODECLS_REGISTER(
	JIVE_BITCONCAT_NODE, "bitconcat",
	jive_bitconcat_serialize,
	jive_bitconcat_deserialize);

JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY1(
	JIVE_BITAND_NODE, jive::bits::and_op, "bitand");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY1(
	JIVE_BITOR_NODE, jive::bits::or_op, "bitor");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY1(
	JIVE_BITXOR_NODE, jive::bits::xor_op, "bitxor");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITUNARY(
	JIVE_BITNOT_NODE, jive::bits::not_op, "bitnot");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITUNARY(
	JIVE_BITNEGATE_NODE, jive::bits::neg_op, "bitnegate");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY2(
	JIVE_BITSHL_NODE, jive::bits::shl_op, "bitshl");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY2(
	JIVE_BITSHR_NODE, jive::bits::shr_op, "bitshr");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY2(
	JIVE_BITASHR_NODE, jive::bits::ashr_op, "bitashr");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY1(
	JIVE_BITSUM_NODE, jive::bits::add_op, "bitsum");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY2(
	JIVE_BITDIFFERENCE_NODE, jive::bits::sub_op, "bitdifference");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY1(
	JIVE_BITPRODUCT_NODE, jive::bits::mul_op, "bitproduct");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY2(
	JIVE_BITSHIPRODUCT_NODE, jive::bits::smulh_op, "bitshiproduct");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY2(
	JIVE_BITUHIPRODUCT_NODE, jive::bits::umulh_op, "bituhiproduct");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY2(
	JIVE_BITSQUOTIENT_NODE, jive::bits::sdiv_op, "bitsquotient");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY2(
	JIVE_BITUQUOTIENT_NODE, jive::bits::udiv_op, "bituquotient");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY2(
	JIVE_BITSMOD_NODE, jive::bits::smod_op, "bitsmod");
JIVE_SERIALIZATION_NODECLS_REGISTER_BITBINARY2(
	JIVE_BITUMOD_NODE, jive::bits::umod_op, "bitumod");

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
	const jive::base::type * type_,
	jive_token_ostream * os)
{
	const jive::bits::type * type = static_cast<const jive::bits::type*>(type_);
	jive_serialize_uint(driver, type->nbits(), os);
}

bool
jive_bitstring_type_deserialize(
	const jive_serialization_typecls * self,
	jive_serialization_driver * driver,
	jive_token_istream * is,
	jive::base::type ** type)
{
	uint64_t nbits;
	if (!jive_deserialize_uint(driver, is, &nbits))
		return false;
	
	/* FIXME: check number of bits */
	
	*type = new jive::bits::type(nbits);
	return true;
}

JIVE_SERIALIZATION_TYPECLS_REGISTER(
	jive::bits::type, jive_bitstring_type, "bits",
	jive_bitstring_type_serialize,
	jive_bitstring_type_deserialize);
