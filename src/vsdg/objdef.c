/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/objdef.h>

#include <string.h>

#include <jive/util/buffer.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/statetype.h>

namespace jive {

objdef_operation::~objdef_operation() noexcept
{
}

bool
objdef_operation::operator==(const operation & other) const noexcept
{
	const objdef_operation * op =
		dynamic_cast<const objdef_operation *>(&other);
	return op &&
		op->name() == name() &&
		op->symbol() == symbol() &&
		*op->type_ == *type_;
}

size_t
objdef_operation::narguments() const noexcept
{
	return 1;
}

const jive::base::type &
objdef_operation::argument_type(size_t index) const noexcept
{
	return *type_;
}

size_t
objdef_operation::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
objdef_operation::result_type(size_t index) const noexcept
{
	/* FIXME: this is horribly wrong, but we don't have another type right
	now for putting in here, this entire node needs to be remodeled */
	return jive::ctl::boolean;
}
std::string
objdef_operation::debug_string() const
{
	return name();
}

std::unique_ptr<jive::operation>
objdef_operation::copy() const
{
	return std::unique_ptr<jive::operation>(new objdef_operation(*this));
}

}

jive::oport *
jive_objdef_create(
	jive::oport * output,
	const char * name,
	const jive_linker_symbol * symbol)
{
	jive::objdef_operation op(name, symbol, output->type());
	return jive_node_create_normalized(output->region(), op, {output})[0];
}
