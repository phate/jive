/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/float/arithmetic/fltdifference.hpp>
#include <jive/types/float/arithmetic/fltnegate.hpp>
#include <jive/types/float/arithmetic/fltproduct.hpp>
#include <jive/types/float/arithmetic/fltquotient.hpp>
#include <jive/types/float/arithmetic/fltsum.hpp>

namespace jive {
namespace flt {

const char fltdifference_name[] = "FLTDIFFERENCE";
const char fltnegate_name[] = "FLTNEGATE";
const char fltproduct_name[] = "FLTPRODUCT";
const char fltquotient_name[] = "FLTQUOTIENT";
const char fltsum_name[] = "FLTSUM";

}
}

jive::output *
jive_fltdifference(jive::output * arg1, jive::output * arg2)
{
	return jive::flt::sub_op::normalized_create(arg1, arg2);
}

jive::output *
jive_fltnegate(jive::output * arg)
{
	return jive::flt::neg_op::normalized_create(arg);
}

jive::output *
jive_fltproduct(jive::output * arg1, jive::output * arg2)
{
	return jive::flt::mul_op::normalized_create(arg1, arg2);
}

jive::output *
jive_fltquotient(jive::output * arg1, jive::output * arg2)
{
	return jive::flt::div_op::normalized_create(arg1, arg2);
}

jive::output *
jive_fltsum(jive::output * arg1, jive::output * arg2)
{
	return jive::flt::add_op::normalized_create(arg1, arg2);
}
