/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/float/comparison/fltequal.hpp>
#include <jive/types/float/comparison/fltgreater.hpp>
#include <jive/types/float/comparison/fltgreatereq.hpp>
#include <jive/types/float/comparison/fltless.hpp>
#include <jive/types/float/comparison/fltlesseq.hpp>
#include <jive/types/float/comparison/fltnotequal.hpp>

namespace jive {
namespace flt {

const char fltequal_name[] = "FLTEQUAL";
const char fltgreater_name[] = "FLTGREATER";
const char fltgreatereq_name[] = "FLTGREATEREQ";
const char fltless_name[] = "FLTLESS";
const char fltlesseq_name[] = "FLTLESSEQ";
const char fltnotequal_name[] = "FLTNOTEQUAL";

}
}

jive::output *
jive_fltequal(jive::output * arg1, jive::output * arg2)
{
	return jive::flt::eq_op::normalized_create(arg1, arg2);
}

jive::output *
jive_fltgreater(jive::output * arg1, jive::output * arg2)
{
	return jive::flt::gt_op::normalized_create(arg1, arg2);
}

jive::output *
jive_fltgreatereq(jive::output * arg1, jive::output * arg2)
{
	return jive::flt::ge_op::normalized_create(arg1, arg2);
}

jive::output *
jive_fltless(jive::output * arg1, jive::output * arg2)
{
	return jive::flt::lt_op::normalized_create(arg1, arg2);
}

jive::output *
jive_fltlesseq(jive::output * arg1, jive::output * arg2)
{
	return jive::flt::le_op::normalized_create(arg1, arg2);
}

jive::output *
jive_fltnotequal(jive::output * arg1, jive::output * arg2)
{
	return jive::flt::ne_op::normalized_create(arg1, arg2);
}
