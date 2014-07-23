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
#include <jive/vsdg/theta.h>

JIVE_SERIALIZATION_OPNODE_REGISTER_SIMPLE(
	JIVE_GAMMA_NODE,
	jive::gamma_op,
	"gamma");
JIVE_SERIALIZATION_OPNODE_REGISTER_SIMPLE(
	JIVE_GAMMA_TAIL_NODE,
	jive::gamma_tail_op,
	"gamma_tail");

JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_THETA_NODE, "theta");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_THETA_HEAD_NODE, "theta_head");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_THETA_TAIL_NODE, "theta_tail");

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

static void
jive_ctlconstant_serialize(
	const jive_serialization_nodecls * self,
	jive_serialization_driver * driver,
	const jive_node_attrs * attrs_, jive_token_ostream * os)
{
	const jive::ctl::constant_op * op = static_cast<const jive::ctl::constant_op *>(attrs_);
	jive_token_ostream_integral(os, op->value() ? 1 : 0);
}

static bool
jive_ctlconstant_deserialize(
	const jive_serialization_nodecls * self,
	jive_serialization_driver * driver,
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[],
	jive_token_istream * is,
	jive_node ** node)
{
	uint64_t value;
	if (!jive_deserialize_uint(driver, is, &value)) {
		return false;
	}
	jive::ctl::constant_op op(!!value);

	*node = op.create_node(region, narguments, arguments);

	return true;
}

JIVE_SERIALIZATION_NODECLS_REGISTER(
	JIVE_CONTROL_CONSTANT_NODE, "ctlconst",
	jive_ctlconstant_serialize,
	jive_ctlconstant_deserialize);

