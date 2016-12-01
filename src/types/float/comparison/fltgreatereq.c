/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/float/comparison/fltgreatereq.h>

#include <jive/types/float/flttype.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>

namespace jive {
namespace flt {

const char fltgreatereq_name[] = "FLTGREATEREQ";

}
}

jive::oport *
jive_fltgreatereq(jive::oport * arg1, jive::oport * arg2)
{
	return jive::flt::ge_op::normalized_create(arg1, arg2);
}
