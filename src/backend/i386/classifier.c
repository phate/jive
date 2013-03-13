/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/backend/i386/classifier.h>

#include <jive/backend/i386/registerset.h>
#include <jive/types/float/flttype.h>
#include <jive/vsdg/basetype.h>

typedef enum jive_i386_classify_regcls {
	jive_i386_classify_flags = 0,
	jive_i386_classify_gpr = 1,
	jive_i386_classify_fp = 2,
	jive_i386_classify_sse = 3,
} jive_i386_classify_regcls;

static const jive_register_class * classes [] =  {
	[jive_i386_classify_flags] = &jive_i386_regcls_flags,
	[jive_i386_classify_gpr] = &jive_i386_regcls_gpr,
	[jive_i386_classify_fp] = &jive_i386_regcls_fp,
	[jive_i386_classify_sse] = &jive_i386_regcls_sse,
};

static jive_regselect_mask
jive_i386_classify_type_(const jive_type * type, const jive_resource_class * rescls)
{
	rescls = jive_resource_class_relax(rescls);
	
	if (rescls == &jive_i386_regcls_gpr.base)
		return (1 << jive_i386_classify_gpr);
	else if (rescls == &jive_i386_regcls_flags.base)
		return (1 << jive_i386_classify_flags);
	
	if (type->class_ == &JIVE_BITSTRING_TYPE) {
		const jive_bitstring_type * btype = (const jive_bitstring_type *) type;
		if (btype->nbits == 32)
			return (1 << jive_i386_classify_gpr);
	}

	if (type->class_ == &JIVE_FLOAT_TYPE) {
		return (1 << jive_i386_classify_sse);
	}
	
	/* no suitable register class */
	/* FIXME: this should *probably* not be a fatal error -- but since
	this is usually indicative of bugs in other parts of the compiler,
	error out here to better expose problems */
	JIVE_DEBUG_ASSERT(false);
	return 0;
}

static jive_regselect_mask
jive_i386_classify_fixed_arithmetic_(jive_bitop_code op, size_t nbits)
{
	return (1 << jive_i386_classify_gpr);
}

static jive_regselect_mask
jive_i386_classify_fixed_compare_(jive_bitcmp_code op, size_t nbits)
{
	return (1 << jive_i386_classify_gpr);
}

static jive_regselect_mask
jive_i386_classify_float_arithmetic_(jive_fltop_code op)
{
	return (1 << jive_i386_classify_sse);
}

static jive_regselect_mask
jive_i386_classify_float_compare_(jive_fltcmp_code op)
{
	return (1 << jive_i386_classify_sse);
}

static jive_regselect_mask
jive_i386_classify_address_(void)
{
	return (1 << jive_i386_classify_gpr);
}

const jive_reg_classifier jive_i386_reg_classifier = {
	.any = (1 << jive_i386_classify_gpr) | (1 << jive_i386_classify_flags),
	.classify_type = jive_i386_classify_type_,
	.classify_fixed_arithmetic = jive_i386_classify_fixed_arithmetic_,
	.classify_float_arithmetic = jive_i386_classify_float_arithmetic_,
	.classify_fixed_compare = jive_i386_classify_fixed_compare_,
	.classify_float_compare = jive_i386_classify_float_compare_,
	.classify_address = jive_i386_classify_address_,
	
	.nclasses = 5,
	.classes = classes,
};
