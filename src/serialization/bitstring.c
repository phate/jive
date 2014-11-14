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

namespace jive {
namespace serialization {
namespace {

class bitconstant_handler : public opcls_handler {
public:
	inline bitconstant_handler(
		std::string tag,
		opcls_registry & registry)
		: opcls_handler(tag, typeid(bits::constant_op), registry)
	{
	}

	virtual void
	serialize(
		const operation & op,
		output_driver & driver) const override
	{
		const bits::constant_op & const_op = static_cast<const bits::constant_op &>(op);
		driver.put_string(const_op.value().str());
	}

	virtual std::unique_ptr<operation>
	deserialize(
		parser_driver & driver) const override
	{
		std::string s = driver.parse_string();
		return std::unique_ptr<operation>(new bits::constant_op(jive::bits::value_repr(s)));
	}
};


template<typename Operation>
class bitbinary1_handler : public opcls_handler {
public:
	inline bitbinary1_handler(
		std::string tag,
		opcls_registry & registry)
		: opcls_handler(tag, typeid(Operation), registry)
	{
	}

	virtual void
	serialize(
		const operation & op,
		output_driver & driver) const override
	{
		driver.put_uint(static_cast<const bits::binary_op &>(op).type().nbits());
		driver.put_char_token(',');
		driver.put_int(static_cast<const bits::binary_op &>(op).narguments());
	}

	virtual std::unique_ptr<operation>
	deserialize(
		parser_driver & driver) const override
	{
		size_t nbits = driver.parse_uint();
		driver.parse_char_token(',');
		size_t nargs = driver.parse_uint();
		return std::unique_ptr<operation>(new Operation(nbits, nargs));
	}
};

template<typename Operation>
class bitbinary2_handler : public opcls_handler {
public:
	inline bitbinary2_handler(
		std::string tag,
		opcls_registry & registry)
		: opcls_handler(tag, typeid(Operation), registry)
	{
	}

	virtual void
	serialize(
		const operation & op,
		output_driver & driver) const override
	{
		driver.put_uint(static_cast<const bits::binary_op &>(op).type().nbits());
	}

	virtual std::unique_ptr<operation>
	deserialize(
		parser_driver & driver) const override
	{
		size_t nbits = driver.parse_uint();
		return std::unique_ptr<operation>(new Operation(nbits));
	}
};

template<typename Operation>
class bitunary_handler : public opcls_handler {
public:
	inline bitunary_handler(
		std::string tag,
		opcls_registry & registry)
		: opcls_handler(tag, typeid(Operation), registry)
	{
	}

	virtual void
	serialize(
		const operation & op,
		output_driver & driver) const override
	{
		driver.put_uint(static_cast<const bits::unary_op &>(op).type().nbits());
	}

	virtual std::unique_ptr<operation>
	deserialize(
		parser_driver & driver) const override
	{
		size_t nbits = driver.parse_uint();
		return std::unique_ptr<operation>(new Operation(nbits));
	}
};

class bitconcat_handler : public opcls_handler {
public:
	inline bitconcat_handler(
		std::string tag,
		opcls_registry & registry)
		: opcls_handler(tag, typeid(bits::concat_op), registry)
	{
	}

	virtual void
	serialize(
		const operation & op,
		output_driver & driver) const override
	{
		bool first = true;
		for (const bits::type & t : static_cast<const bits::concat_op&>(op).argument_types()) {
			if (!first) {
				driver.put_char_token(',');
			} else {
				first = false;
			}
			driver.put_uint(t.nbits());
		}
	}

	virtual std::unique_ptr<operation>
	deserialize(
		parser_driver & driver) const override
	{
		std::vector<bits::type> types;
		
		for (;;) {
			uint64_t bits = driver.parse_uint();
			types.emplace_back(bits);
			if (jive_token_istream_current(&driver.istream())->type == jive_token_comma) {
				jive_token_istream_advance(&driver.istream());
			} else {
				break;
			}
		}

		return std::unique_ptr<operation>(new bits::concat_op(std::move(types)));
	}
};

class bitslice_handler : public opcls_handler {
public:
	inline bitslice_handler(
		std::string tag,
		opcls_registry & registry)
		: opcls_handler(tag, typeid(bits::slice_op), registry)
	{
	}

	virtual void
	serialize(
		const operation & op,
		output_driver & driver) const override
	{
		const bits::slice_op & slice_op = static_cast<const bits::slice_op &>(op);
		driver.put_uint(slice_op.argument_type().nbits());
		driver.put_char_token(',');
		driver.put_uint(slice_op.low());
		driver.put_char_token(',');
		driver.put_uint(slice_op.high());
	}

	virtual std::unique_ptr<operation>
	deserialize(
		parser_driver & driver) const override
	{
		size_t nbits = driver.parse_uint();
		driver.parse_char_token(',');
		size_t low = driver.parse_uint();
		driver.parse_char_token(',');
		size_t high  = driver.parse_uint();

		return std::unique_ptr<operation>(new bits::slice_op(nbits, low, high));
	}
};

#define JIVE_SERIALIZATION_OPCLS_REGISTER_BITBINARY1(tag, opcls) \
	bitbinary1_handler<opcls> register_cls_##tag(#tag, opcls_registry::mutable_instance());

#define JIVE_SERIALIZATION_OPCLS_REGISTER_BITBINARY2(tag, opcls) \
	bitbinary2_handler<opcls> register_cls_##tag(#tag, opcls_registry::mutable_instance());

#define JIVE_SERIALIZATION_OPCLS_REGISTER_BITUNARY(tag, opcls) \
	bitunary_handler<opcls> register_cls_##tag(#tag, opcls_registry::mutable_instance());

bitconstant_handler register_cls_bitconstant("bitconstant", opcls_registry::mutable_instance());
bitconcat_handler register_cls_bitconcat("bitconcat", opcls_registry::mutable_instance());
bitslice_handler register_cls_bitslice("bitslice", opcls_registry::mutable_instance());

JIVE_SERIALIZATION_OPCLS_REGISTER_BITBINARY1(bitand, jive::bits::and_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_BITBINARY1(bitor, jive::bits::or_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_BITBINARY1(bitxor, jive::bits::xor_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_BITUNARY(bitnot, jive::bits::not_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_BITUNARY(bitnegate, jive::bits::neg_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_BITBINARY2(bitshl, jive::bits::shl_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_BITBINARY2(bitshr, jive::bits::shr_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_BITBINARY2(bitashr, jive::bits::ashr_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_BITBINARY1(bitsum, jive::bits::add_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_BITBINARY2(bitdifference, jive::bits::sub_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_BITBINARY1(bitproduct, jive::bits::mul_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_BITBINARY2(bitshiproduct, jive::bits::smulh_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_BITBINARY2(bituhiproduct, jive::bits::umulh_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_BITBINARY2(bitsquotient, jive::bits::sdiv_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_BITBINARY2(bituquotient, jive::bits::udiv_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_BITBINARY2(bitsmod, jive::bits::smod_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_BITBINARY2(bitumod, jive::bits::umod_op);

JIVE_SERIALIZATION_OPCLS_REGISTER_BITBINARY2(bitequal, jive::bits::eq_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_BITBINARY2(bitnotequal, jive::bits::ne_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_BITBINARY2(bitsgreater, jive::bits::sgt_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_BITBINARY2(bitsgreatereq, jive::bits::sge_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_BITBINARY2(bitugreater, jive::bits::ugt_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_BITBINARY2(bitugreatereq, jive::bits::uge_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_BITBINARY2(bitsless, jive::bits::slt_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_BITBINARY2(bitslesseq, jive::bits::sle_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_BITBINARY2(bituless, jive::bits::ult_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_BITBINARY2(bitulesseq, jive::bits::ule_op);

}
}
}


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
