/*
 * Copyright 2017 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/common.h>
#include <jive/vsdg/operators/structural.h>

namespace jive {

bool
structural_op::operator==(const operation & other) const noexcept
{
	return typeid(*this) == typeid(other);
}

size_t
structural_op::narguments() const noexcept
{
	return 0;
}

const jive::base::type &
structural_op::argument_type(size_t index) const noexcept
{
	JIVE_ASSERT(0 && "structural_op has no arguments.");
}

size_t
structural_op::nresults() const noexcept
{
	return 0;
}

const jive::base::type &
structural_op::result_type(size_t index) const noexcept
{
	JIVE_ASSERT(0 && "structural_op has no arguments.");
}

}
