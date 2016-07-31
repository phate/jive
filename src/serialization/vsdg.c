/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/serialization/driver.h>
#include <jive/serialization/grammar.h>
#include <jive/serialization/nodecls-registry.h>
#include <jive/serialization/rescls-registry.h>
#include <jive/serialization/token-stream.h>
#include <jive/serialization/typecls-registry.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/gamma.h>
#include <jive/vsdg/resource.h>
#include <jive/vsdg/seqtype.h>
#include <jive/vsdg/splitnode.h>
#include <jive/vsdg/theta.h>

static void
jive_control_type_serialize(
	const jive_serialization_typecls * self,
	jive_serialization_driver * driver,
	const jive::base::type * type_,
	jive_token_ostream * os)
{
	const jive::ctl::type * type = static_cast<const jive::ctl::type*>(type_);
	jive_serialize_uint(driver, type->nalternatives(), os);
}

static bool
jive_control_type_deserialize(
	const jive_serialization_typecls * self,
	jive_serialization_driver * driver,
	jive_token_istream * is,
	jive::base::type ** type)
{
	uint64_t nalternatives;
	if (!jive_deserialize_uint(driver, is, &nalternatives))
		return false;

	*type = new jive::ctl::type(nalternatives);
	return true;
}

JIVE_SERIALIZATION_TYPECLS_REGISTER(jive::ctl::type, jive_control_type, "control",
	jive_control_type_serialize, jive_control_type_deserialize);

static void
jive_seq_type_serialize(
	const jive_serialization_typecls * self,
	jive_serialization_driver * driver,
	const jive::base::type * type,
	jive_token_ostream * os)
{
	/* no attributes */
}

static bool
jive_seq_type_deserialize(
	const jive_serialization_typecls * self,
	jive_serialization_driver * driver,
	jive_token_istream * is,
	jive::base::type ** type)
{
	*type = new jive::seq::type();
	return true;
}

JIVE_SERIALIZATION_TYPECLS_REGISTER(jive::seq::type, jive_seq_type, "seq",
	jive_seq_type_serialize, jive_seq_type_deserialize);

JIVE_SERIALIZATION_RESCLS_REGISTER(jive_root_resource_class, "root");

namespace jive {
namespace serialization {
namespace {

class ctlconst_handler final : public opcls_handler {
public:
	inline ctlconst_handler(
		std::string tag,
		opcls_registry & registry)
		: opcls_handler(tag, typeid(ctl::constant_op), registry)
	{
	}

	virtual void
	serialize(
		const operation & op_,
		output_driver & driver) const override
	{
		const ctl::constant_op & op = static_cast<const ctl::constant_op&>(op_);
		driver.put_uint(op.value().nalternatives());
		driver.put_char_token(',');
		driver.put_uint(op.value().alternative());
	}

	virtual std::unique_ptr<operation>
	deserialize(
		parser_driver & driver) const override
	{
		uint64_t nalternatives = driver.parse_uint();
		driver.parse_char_token(',');
		uint64_t alternative = driver.parse_uint();
		return std::unique_ptr<operation>(new ctl::constant_op(
			ctl::value_repr(alternative, nalternatives)));
	}
};

class split_handler final : public opcls_handler {
public:
	inline split_handler(
		std::string tag,
		opcls_registry & registry)
		: opcls_handler(tag, typeid(split_operation), registry)
	{
	}

	virtual void
	serialize(
		const operation & op,
		output_driver & driver) const override
	{
		const split_operation & s_op = static_cast<const split_operation &>(op);
		driver.put_resource_class(s_op.in_class());
		driver.put_char_token(',');
		driver.put_resource_class(s_op.out_class());
	}

	virtual std::unique_ptr<operation>
	deserialize(
		parser_driver & driver) const override
	{
		const jive_resource_class * in_class =
			driver.parse_resource_class();
		driver.parse_char_token(',');
		const jive_resource_class * out_class =
			driver.parse_resource_class();
		return std::unique_ptr<operation>(new split_operation(in_class, out_class));
	}
};

class gamma_op_handler final : public opcls_handler {
public:
	inline
	gamma_op_handler(std::string tag, opcls_registry & registry)
	: opcls_handler(tag, typeid(gamma_op), registry)
	{}

	virtual void
	serialize(const operation & op_, output_driver & driver) const override
	{
		const gamma_op & op = static_cast<const gamma_op &>(op_);
		driver.put_uint(op.nalternatives());
	}

	virtual std::unique_ptr<operation>
	deserialize(parser_driver & driver) const override
	{
		size_t nalternatives = driver.parse_uint();
		return std::unique_ptr<operation>(new gamma_op(nalternatives));
	}
};

ctlconst_handler registerer_ctlconst("ctlconst", opcls_registry::mutable_instance());
split_handler registerer_split_op("split", opcls_registry::mutable_instance());
JIVE_SERIALIZATION_OPCLS_REGISTER_SIMPLE(graph_tail, graph_tail_operation);
gamma_op_handler register_gamma_op("gamma", opcls_registry::mutable_instance());
JIVE_SERIALIZATION_OPCLS_REGISTER_SIMPLE(gamma_head, gamma_head_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_SIMPLE(gamma_tail, gamma_tail_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_SIMPLE(theta, theta_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_SIMPLE(theta_head, theta_head_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_SIMPLE(theta_tail, theta_tail_op);

}
}
}
