/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/float/arithmetic/fltdifference.h>

#include <jive/types/float/flttype.h>
#include <jive/vsdg/graph.h>

namespace jive {
namespace flt {

const char fltdifference_name[] = "FLTDIFFERENCE";

}
}

jive::oport *
jive_fltdifference(jive::oport * arg1, jive::oport * arg2)
{
	return jive::flt::sub_op::normalized_create(arg1, arg2);
}
