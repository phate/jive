/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/float/arithmetic/fltdifference.h>
#include <jive/types/float/arithmetic/fltnegate.h>
#include <jive/types/float/arithmetic/fltproduct.h>
#include <jive/types/float/arithmetic/fltquotient.h>
#include <jive/types/float/arithmetic/fltsum.h>

namespace jive {
namespace flt {

const char fltdifference_name[] = "FLTDIFFERENCE";
const char fltnegate_name[] = "FLTNEGATE";
const char fltproduct_name[] = "FLTPRODUCT";
const char fltquotient_name[] = "FLTQUOTIENT";
const char fltsum_name[] = "FLTSUM";

}
}

jive::oport *
jive_fltdifference(jive::oport * arg1, jive::oport * arg2)
{
	return jive::flt::sub_op::normalized_create(arg1, arg2);
}

jive::oport *
jive_fltnegate(jive::oport * arg)
{
	return jive::flt::neg_op::normalized_create(arg);
}

jive::oport *
jive_fltproduct(jive::oport * arg1, jive::oport * arg2)
{
	return jive::flt::mul_op::normalized_create(arg1, arg2);
}

jive::oport *
jive_fltquotient(jive::oport * arg1, jive::oport * arg2)
{
	return jive::flt::div_op::normalized_create(arg1, arg2);
}

jive::oport *
jive_fltsum(jive::oport * arg1, jive::oport * arg2)
{
	return jive::flt::add_op::normalized_create(arg1, arg2);
}
