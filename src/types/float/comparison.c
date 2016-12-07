/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/float/comparison/fltequal.h>
#include <jive/types/float/comparison/fltgreater.h>
#include <jive/types/float/comparison/fltgreatereq.h>
#include <jive/types/float/comparison/fltless.h>
#include <jive/types/float/comparison/fltlesseq.h>
#include <jive/types/float/comparison/fltnotequal.h>

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

jive::oport *
jive_fltequal(jive::oport * arg1, jive::oport * arg2)
{
	return jive::flt::eq_op::normalized_create(arg1, arg2);
}

jive::oport *
jive_fltgreater(jive::oport * arg1, jive::oport * arg2)
{
	return jive::flt::gt_op::normalized_create(arg1, arg2);
}

jive::oport *
jive_fltgreatereq(jive::oport * arg1, jive::oport * arg2)
{
	return jive::flt::ge_op::normalized_create(arg1, arg2);
}

jive::oport *
jive_fltless(jive::oport * arg1, jive::oport * arg2)
{
	return jive::flt::lt_op::normalized_create(arg1, arg2);
}

jive::oport *
jive_fltlesseq(jive::oport * arg1, jive::oport * arg2)
{
	return jive::flt::le_op::normalized_create(arg1, arg2);
}

jive::oport *
jive_fltnotequal(jive::oport * arg1, jive::oport * arg2)
{
	return jive::flt::ne_op::normalized_create(arg1, arg2);
}
