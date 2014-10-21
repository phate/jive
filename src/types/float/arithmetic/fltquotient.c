/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/float/arithmetic/fltquotient.h>

#include <jive/types/float/flttype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>

namespace jive {
namespace flt {

const char fltquotient_name[] = "FLTQUOTIENT";

}
}

jive::output *
jive_fltquotient(jive::output * arg1, jive::output * arg2)
{
	return jive::flt::div_op::normalized_create(arg1, arg2);
}
