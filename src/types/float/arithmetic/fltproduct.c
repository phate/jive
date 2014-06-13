/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/float/arithmetic/fltproduct.h>
#include <jive/types/float/fltoperation-classes-private.h>

#include <jive/types/float/flttype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>

namespace jive {
namespace flt {

value_repr compute_product(value_repr arg1, value_repr arg2)
{
	return arg1 * arg2;
}

const char fltproduct_name[] = "FLTPRODUCT";

}
}

const jive_node_class JIVE_FLTPRODUCT_NODE = {
	parent : &JIVE_FLTBINARY_NODE,
	name : "FLTPRODUCT",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_fltproduct(jive::output * arg1, jive::output * arg2)
{
	return jive::flt::product_operation::normalized_create(arg1, arg2);
}
