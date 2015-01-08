/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/address.h>
#include <jive/arch/immediate-node.h>
#include <jive/arch/instruction.h>
#include <jive/arch/memorytype.h>
#include <jive/arch/registers.h>
#include <jive/arch/stackslot.h>
#include <jive/arch/subroutine.h>
#include <jive/arch/subroutine/nodes.h>
#include <jive/serialization/driver.h>
#include <jive/serialization/grammar.h>
#include <jive/serialization/instrcls-registry.h>
#include <jive/serialization/nodecls-registry.h>
#include <jive/serialization/rescls-registry.h>
#include <jive/serialization/token-stream.h>
#include <jive/serialization/typecls-registry.h>

static void
jive_serialization_stackslot_serialize(
	const jive_serialization_rescls * self,
	jive_serialization_driver * driver,
	const jive_resource_class * rescls,
	jive_token_ostream * os)
{
	const jive_stackslot_size_class * cls = (const jive_stackslot_size_class *) rescls;
	
	jive_serialize_uint(driver, cls->size, os);
	jive_serialize_char_token(driver, ',', os);
	jive_serialize_uint(driver, cls->alignment, os);
}

static bool
jive_serialization_stackslot_deserialize(
	const jive_serialization_rescls * self,
	jive_serialization_driver * driver,
	jive_token_istream * is,
	const jive_resource_class ** rescls)
{
	uint64_t size, alignment;
	
	if (!jive_deserialize_uint(driver, is, &size))
		return false;
	if (!jive_deserialize_char_token(driver, is, ','))
		return false;
	if (!jive_deserialize_uint(driver, is, &alignment))
		return false;
	
	*rescls = jive_stackslot_size_class_get(size, alignment);
	
	return *rescls != 0;
}

static void
jive_serialization_frameslot_serialize(
	const jive_serialization_rescls * self,
	jive_serialization_driver * driver,
	const jive_resource_class * rescls,
	jive_token_ostream * os)
{
	const jive_fixed_stackslot_class * cls = (const jive_fixed_stackslot_class *) rescls;
	const jive_stackslot * slot = (const jive_stackslot *) cls->slot;
	
	jive_serialize_uint(driver, cls->base.size, os);
	jive_serialize_char_token(driver, ',', os);
	jive_serialize_uint(driver, cls->base.alignment, os);
	jive_serialize_char_token(driver, ',', os);
	jive_serialize_int(driver, slot->offset, os);
}


static bool
jive_serialization_frameslot_deserialize(
	const jive_serialization_rescls * self,
	jive_serialization_driver * driver,
	jive_token_istream * is,
	const jive_resource_class ** rescls)
{
	uint64_t size, alignment;
	int64_t offset;
	
	if (!jive_deserialize_uint(driver, is, &size))
		return false;
	if (!jive_deserialize_char_token(driver, is, ','))
		return false;
	if (!jive_deserialize_uint(driver, is, &alignment))
		return false;
	if (!jive_deserialize_char_token(driver, is, ','))
		return false;
	if (!jive_deserialize_int(driver, is, &offset))
		return false;
	
	*rescls = jive_fixed_stackslot_class_get(size, alignment, offset);
	
	return *rescls != 0;
}

static void
jive_serialization_callslot_serialize(
	const jive_serialization_rescls * self,
	jive_serialization_driver * driver,
	const jive_resource_class * rescls,
	jive_token_ostream * os)
{
	const jive_callslot_class * cls = (const jive_callslot_class *) rescls;
	const jive_callslot * slot = (const jive_callslot *) cls->slot;
	
	jive_serialize_uint(driver, cls->base.size, os);
	jive_serialize_char_token(driver, ',', os);
	jive_serialize_uint(driver, cls->base.alignment, os);
	jive_serialize_char_token(driver, ',', os);
	jive_serialize_int(driver, slot->offset, os);
}

static bool
jive_serialization_callslot_deserialize(
	const jive_serialization_rescls * self,
	jive_serialization_driver * driver,
	jive_token_istream * is,
	const jive_resource_class ** rescls)
{
	uint64_t size, alignment;
	int64_t offset;
	
	if (!jive_deserialize_uint(driver, is, &size))
		return false;
	if (!jive_deserialize_char_token(driver, is, ','))
		return false;
	if (!jive_deserialize_uint(driver, is, &alignment))
		return false;
	if (!jive_deserialize_char_token(driver, is, ','))
		return false;
	if (!jive_deserialize_int(driver, is, &offset))
		return false;
	
	*rescls = jive_callslot_class_get(size, alignment, offset);
	
	return *rescls != 0;
}

static void
jive_memory_type_serialize(
	const jive_serialization_typecls * self,
	jive_serialization_driver * driver,
	const jive::base::type * type,
	jive_token_ostream * os)
{
	/* no attributes */
}

static bool
jive_memory_type_deserialize(
	const jive_serialization_typecls * self,
	jive_serialization_driver * driver,
	jive_token_istream * is,
	jive::base::type ** type)
{
	*type = new jive::mem::type();
	return true;
}

namespace jive {
namespace serialization {
namespace {

class immediate_handler final : public opcls_handler {
public:
	inline immediate_handler(
		std::string tag,
		opcls_registry & registry)
		: opcls_handler(tag, typeid(immediate_op), registry)
	{
	}

	virtual void
	serialize(
		const operation & op,
		output_driver & driver) const override
	{
		const immediate_op & imm_op = static_cast<const immediate_op &>(op);
		driver.put_uint(imm_op.value().offset);
		if (imm_op.value().add_label) {
			driver.put_char_token('+');
			driver.put_label(imm_op.value().add_label);
		}
		if (imm_op.value().sub_label) {
			driver.put_char_token('-');
			driver.put_label(imm_op.value().sub_label);
		}
	}

	virtual std::unique_ptr<operation>
	deserialize(
		parser_driver & driver) const override
	{
		jive_immediate imm;
		jive_immediate_init(&imm, 0, nullptr, nullptr, nullptr);

		imm.offset = driver.parse_uint();

		if (driver.peek_token_type() == jive_token_plus) {
			driver.parse_char_token('+');
			imm.add_label = driver.parse_label();
		}

		if (driver.peek_token_type() == jive_token_plus) {
			driver.parse_char_token('-');
			imm.sub_label = driver.parse_label();
		}

		return std::unique_ptr<operation>(new immediate_op(imm));
	}
};

class instr_handler final : public opcls_handler {
public:
	inline instr_handler(
		std::string tag,
		opcls_registry & registry)
		: opcls_handler(tag, typeid(instruction_op), registry)
	{
	}

	virtual void
	serialize(
		const operation & op,
		output_driver & driver) const override
	{
		const jive::instruction_op & i_op = static_cast<const jive::instruction_op &>(op);
		const jive_serialization_instrcls_registry * reg = driver.driver().instrcls_registry;
		const jive_serialization_instrcls * sercls =
			jive_serialization_instrcls_lookup_by_cls(reg, i_op.icls());

		driver.put_identifier(sercls->tag);
		driver.put_char_token('<');

		/* input states */
		driver.put_char_token('<');
		const std::vector<std::unique_ptr<jive::state::type>> & istates = i_op.istates();
		for (size_t n = 0; n < istates.size(); n++) {
			if (n != 0)
				driver.put_char_token(',');
			jive_serialize_type(&driver.driver(), istates[n].get(), &driver.ostream());
		}
		driver.put_char_token('>');

		driver.put_char_token(',');

		/* output states */
		driver.put_char_token('<');
		const std::vector<std::unique_ptr<jive::state::type>> & ostates = i_op.ostates();
		for (size_t n = 0; n < ostates.size(); n++) {
			if (n != 0)
				driver.put_char_token(',');
			jive_serialize_type(&driver.driver(), ostates[n].get(), &driver.ostream());
		}
		driver.put_char_token('>');

		driver.put_char_token('>');
	}

	virtual std::unique_ptr<operation>
	deserialize(
		parser_driver & driver) const override
	{
		const jive_token * token = jive_token_istream_current(&driver.istream());
		std::string tag = driver.parse_identifier();
		
		const jive_serialization_instrcls * sercls =
			jive_serialization_instrcls_lookup_by_tag(
				driver.driver().instrcls_registry, token->v.identifier);
		if (!sercls) {
			throw parse_error("Expected instruction class identifier");
		}

		driver.parse_char_token('<');

		/* input states */
		driver.parse_char_token('<');
		std::vector<std::unique_ptr<jive::state::type>> istates;
		for (;;) {
			if (driver.peek_token_type() == jive_token_gt)
				break;

			jive::base::type * type;
			if (!jive_deserialize_type(&driver.driver(), &driver.istream(), &type))
				throw parse_error("Expected type");

			if (!dynamic_cast<jive::state::type*>(type)) {
				delete type;
				throw parse_error("Expected state type");
			} else
				istates.emplace_back(static_cast<const jive::state::type*>(type)->copy());

			if (driver.peek_token_type() == jive_token_comma)
				driver.parse_char_token(',');
		}
		driver.parse_char_token('>');

		driver.parse_char_token(',');

		/* output states */
		driver.parse_char_token('<');
		std::vector<std::unique_ptr<jive::state::type>> ostates;
		for (;;) {
			if (driver.peek_token_type() == jive_token_gt)
				break;

			jive::base::type * type;
			if (!jive_deserialize_type(&driver.driver(), &driver.istream(), &type))
				throw parse_error("Expected type");

			if (!dynamic_cast<jive::state::type*>(type)) {
				delete type;
				throw parse_error("Expected state type");
			} else
				ostates.emplace_back(static_cast<const jive::state::type*>(type)->copy());

			if (driver.peek_token_type() == jive_token_comma)
				driver.parse_char_token(',');
		}
		driver.parse_char_token('>');

		driver.parse_char_token('>');

		return std::unique_ptr<operation>(new instruction_op(sercls->cls, istates, ostates));
	}
};

class subroutine_handler final : public opcls_handler {
public:
	inline subroutine_handler(
		std::string tag,
		opcls_registry & registry)
		: opcls_handler(tag, typeid(subroutine_op), registry)
	{
	}

	virtual void
	serialize(
		const operation & op,
		output_driver & driver) const override
	{
		const subroutine_op & s_op = static_cast<const subroutine_op &>(op);
		const subroutine_machine_signature & sig = s_op.signature();
		driver.put_char_token('<');
		bool first = true;
		for (const auto arg : sig.arguments) {
			if (!first) {
				driver.put_char_token(',');
			}
			first = false;
			driver.put_char_token('<');
			driver.put_string(arg.name);
			driver.put_char_token(',');
			driver.put_resource_class_or_null(arg.rescls);
			driver.put_char_token(',');
			driver.put_uint(arg.may_spill);
			driver.put_char_token('>');
		}
		driver.put_char_token('>');
		driver.put_char_token(',');
		driver.put_char_token('<');
		first = true;
		for (const auto pt : sig.passthroughs) {
			if (!first) {
				driver.put_char_token(',');
			}
			first = false;
			driver.put_char_token('<');
			driver.put_string(pt.name);
			driver.put_char_token(',');
			driver.put_resource_class_or_null(pt.rescls);
			driver.put_char_token(',');
			driver.put_uint(pt.may_spill);
			driver.put_char_token('>');
		}
		driver.put_char_token('>');
		driver.put_char_token(',');
		driver.put_char_token('<');
		first = true;
		for (const auto res : sig.results) {
			if (!first) {
				driver.put_char_token(',');
			}
			first = false;
			driver.put_char_token('<');
			driver.put_string(res.name);
			driver.put_char_token(',');
			driver.put_resource_class_or_null(res.rescls);
			driver.put_char_token('>');
		}
		driver.put_char_token('>');
	}

	virtual std::unique_ptr<operation>
	deserialize(
		parser_driver & driver) const override
	{
		subroutine_machine_signature sig;
		driver.parse_char_token('<');
		for (;;) {
			subroutine_machine_signature::argument arg;
			driver.parse_char_token('<');
			arg.name = driver.parse_string();
			driver.parse_char_token(',');
			arg.rescls = driver.parse_resource_class_or_null();
			driver.parse_char_token(',');
			arg.may_spill = driver.parse_uint();
			driver.parse_char_token('>');
			sig.arguments.emplace_back(std::move(arg));
			if (driver.peek_token_type() == jive_token_comma) {
				driver.parse_char_token(',');
			} else {
				break;
			}
		}
		driver.parse_char_token('>');
		driver.parse_char_token(',');
		driver.parse_char_token('<');
		for (;;) {
			subroutine_machine_signature::passthrough pt;
			driver.parse_char_token('<');
			pt.name = driver.parse_string();
			driver.parse_char_token(',');
			pt.rescls = driver.parse_resource_class_or_null();
			driver.parse_char_token(',');
			pt.may_spill = driver.parse_uint();
			driver.parse_char_token('>');
			sig.passthroughs.emplace_back(std::move(pt));
			if (driver.peek_token_type() == jive_token_comma) {
				driver.parse_char_token(',');
			} else {
				break;
			}
		}
		driver.parse_char_token('>');
		driver.parse_char_token(',');
		driver.parse_char_token('<');
		for (;;) {
			subroutine_machine_signature::result res;
			driver.parse_char_token('<');
			res.name = driver.parse_string();
			driver.parse_char_token(',');
			res.rescls = driver.parse_resource_class_or_null();
			driver.parse_char_token('>');
			sig.results.emplace_back(std::move(res));
			if (driver.peek_token_type() == jive_token_comma) {
				driver.parse_char_token(',');
			} else {
				break;
			}
		}
		driver.parse_char_token('>');
		return std::unique_ptr<operation>(new subroutine_op(std::move(sig)));
	}
};

class label2addr_handler final : public opcls_handler {
public:
	inline label2addr_handler(
		std::string tag,
		opcls_registry & registry)
		: opcls_handler(tag, typeid(address::label_to_address_op), registry)
	{
	}

	virtual void
	serialize(
		const operation & op,
		output_driver & driver) const override
	{
		const address::label_to_address_op & l_op =
			static_cast<const address::label_to_address_op &>(op);
		driver.put_label(l_op.label());
	}

	virtual std::unique_ptr<operation>
	deserialize(
		parser_driver & driver) const override
	{
		const jive_label * label = driver.parse_label();
		return std::unique_ptr<operation>(
			new address::label_to_address_op (label));
	}
};

class label2bits_handler final : public opcls_handler {
public:
	inline label2bits_handler(
		std::string tag,
		opcls_registry & registry)
		: opcls_handler(tag, typeid(address::label_to_bitstring_op), registry)
	{
	}

	virtual void
	serialize(
		const operation & op,
		output_driver & driver) const override
	{
		const address::label_to_bitstring_op & l_op =
			static_cast<const address::label_to_bitstring_op &>(op);
		driver.put_label(l_op.label());
		driver.put_char_token(',');
		driver.put_uint(l_op.nbits());
	}

	virtual std::unique_ptr<operation>
	deserialize(
		parser_driver & driver) const override
	{
		const jive_label * label = driver.parse_label();
		driver.parse_char_token(',');
		size_t nbits = driver.parse_uint();
		return std::unique_ptr<operation>(
			new address::label_to_bitstring_op (label, nbits));
	}
};

immediate_handler register_cls_immediate("immediate", opcls_registry::mutable_instance());
instr_handler register_cls_instruction("instr", opcls_registry::mutable_instance());
subroutine_handler register_cls_subroutine("subroutine", opcls_registry::mutable_instance());
JIVE_SERIALIZATION_OPCLS_REGISTER_SIMPLE(subroutine_head, subroutine_head_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_SIMPLE(subroutine_tail, subroutine_tail_op);
label2addr_handler register_cls_label2addr("label2addr", opcls_registry::mutable_instance());
label2addr_handler register_cls_label2bits("label2bits", opcls_registry::mutable_instance());

}
}
}

JIVE_SERIALIZATION_RESCLS_REGISTER(jive_root_register_class, "register");
JIVE_SERIALIZATION_META_RESCLS_REGISTER(JIVE_STACK_RESOURCE, "stackslot",
	jive_serialization_stackslot_serialize,
	jive_serialization_stackslot_deserialize);
JIVE_SERIALIZATION_META_RESCLS_REGISTER(JIVE_STACK_FRAMESLOT_RESOURCE, "stack_frameslot",
	jive_serialization_frameslot_serialize,
	jive_serialization_frameslot_deserialize);
JIVE_SERIALIZATION_META_RESCLS_REGISTER(JIVE_STACK_CALLSLOT_RESOURCE, "stack_callslot",
	jive_serialization_callslot_serialize,
	jive_serialization_callslot_deserialize);
JIVE_SERIALIZATION_TYPECLS_REGISTER(jive::mem::type, jive_memory_type, "memory",
	jive_memory_type_serialize,
	jive_memory_type_deserialize);
