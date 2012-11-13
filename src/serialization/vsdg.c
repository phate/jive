/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/serialization/driver.h>
#include <jive/serialization/nodecls-registry.h>
#include <jive/serialization/rescls-registry.h>
#include <jive/serialization/typecls-registry.h>
#include <jive/serialization/token-stream.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/gamma.h>
#include <jive/vsdg/resource.h>
#include <jive/vsdg/theta.h>

JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_GAMMA_NODE, "gamma");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_GAMMA_TAIL_NODE, "gamma_tail");

JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_THETA_NODE, "theta");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_THETA_HEAD_NODE, "theta_head");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_THETA_TAIL_NODE, "theta_tail");

JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_CONTROL_FALSE_NODE, "control_false");
JIVE_SERIALIZATION_NODECLS_REGISTER_SIMPLE(JIVE_CONTROL_TRUE_NODE, "control_true");

static void
jive_control_type_serialize(
	const jive_serialization_typecls * self,
	jive_serialization_driver * driver,
	const jive_type * type,
	jive_token_ostream * os)
{
	/* no attributes */
}

static bool
jive_control_type_deserialize(
	const jive_serialization_typecls * self,
	jive_serialization_driver * driver,
	jive_token_istream * is,
	jive_type ** type)
{
	JIVE_DECLARE_CONTROL_TYPE(ctl);
	*type = jive_type_copy(ctl, driver->context);
	return true;
}

JIVE_SERIALIZATION_TYPECLS_REGISTER(JIVE_CONTROL_TYPE, "control", jive_control_type_serialize, jive_control_type_deserialize);

JIVE_SERIALIZATION_RESCLS_REGISTER(jive_root_resource_class, "root");
