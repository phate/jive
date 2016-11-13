/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/float/arithmetic/fltnegate.h>

#include <jive/types/float/flttype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>

namespace jive {
namespace flt {

const char fltnegate_name[] = "FLTNEGATE";

}
}

jive::oport *
jive_fltnegate(jive::oport * arg)
{
	return jive::flt::neg_op::normalized_create(arg);
}
