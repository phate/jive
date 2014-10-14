/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
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
#include <jive/vsdg/splitnode.h>
#include <jive/vsdg/theta.h>

static void
jive_control_type_serialize(
	const jive_serialization_typecls * self,
	jive_serialization_driver * driver,
	const jive::base::type * type,
	jive_token_ostream * os)
{
	/* no attributes */
}

static bool
jive_control_type_deserialize(
	const jive_serialization_typecls * self,
	jive_serialization_driver * driver,
	jive_token_istream * is,
	jive::base::type ** type)
{
	*type = new jive::ctl::type();
	return true;
}

JIVE_SERIALIZATION_TYPECLS_REGISTER(jive::ctl::type, jive_control_type, "control",
	jive_control_type_serialize, jive_control_type_deserialize);

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
		const operation & op,
		output_driver & driver) const override
	{
		driver.put_uint(static_cast<const ctl::constant_op &>(op).value() ? 1 : 0);
	}

	virtual std::unique_ptr<operation>
	deserialize(
		parser_driver & driver) const override
	{
		return std::unique_ptr<operation>(new ctl::constant_op(!!driver.parse_uint()));
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

ctlconst_handler registerer_ctlconst("ctlconst", opcls_registry::mutable_instance());
split_handler registerer_split_op("split", opcls_registry::mutable_instance());
JIVE_SERIALIZATION_OPCLS_REGISTER_SIMPLE(graph_tail, graph_tail_operation);
JIVE_SERIALIZATION_OPCLS_REGISTER_SIMPLE(gamma, gamma_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_SIMPLE(gamma_tail, gamma_tail_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_SIMPLE(theta, theta_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_SIMPLE(theta_head, theta_head_op);
JIVE_SERIALIZATION_OPCLS_REGISTER_SIMPLE(theta_tail, theta_tail_op);

}
}
}
