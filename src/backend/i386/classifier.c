/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
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

jive_i386_reg_classifier::~jive_i386_reg_classifier() noexcept
{
}

jive_regselect_mask
jive_i386_reg_classifier::classify_any() const
{
	return (1 << jive_i386_classify_gpr) | (1 << jive_i386_classify_flags);
}

jive_regselect_mask
jive_i386_reg_classifier::classify_type(
	const jive::base::type * type,
	const jive_resource_class * rescls) const
{
	rescls = jive_resource_class_relax(rescls);
	
	if (rescls == &jive_i386_regcls_gpr)
		return (1 << jive_i386_classify_gpr);
	else if (rescls == &jive_i386_regcls_flags)
		return (1 << jive_i386_classify_flags);

	const jive::bits::type * btype = dynamic_cast<const jive::bits::type*>(type);
	if (btype != nullptr) {
		if (btype->nbits() == 32)
			return (1 << jive_i386_classify_gpr);
	}

	if (dynamic_cast<const jive::flt::type*>(type)) {
		return (1 << jive_i386_classify_sse);
	}
	
	/* no suitable register class */
	/* FIXME: this should *probably* not be a fatal error -- but since
	this is usually indicative of bugs in other parts of the compiler,
	error out here to better expose problems */
	JIVE_DEBUG_ASSERT(false);
	return 0;
}

jive_regselect_mask
jive_i386_reg_classifier::classify_fixed_unary(
	const jive::bits::unary_op & op) const
{
	return (1 << jive_i386_classify_gpr);
}

jive_regselect_mask
jive_i386_reg_classifier::classify_fixed_binary(
	const jive::bits::binary_op & op) const
{
	return (1 << jive_i386_classify_gpr);
}

jive_regselect_mask
jive_i386_reg_classifier::classify_fixed_compare(const jive::bits::compare_op & op) const
{
	return (1 << jive_i386_classify_gpr);
}

jive_regselect_mask
jive_i386_reg_classifier::classify_float_unary(const jive::flt::unary_op & op) const
{
	return (1 << jive_i386_classify_sse);
}

jive_regselect_mask
jive_i386_reg_classifier::classify_float_binary(const jive::flt::binary_op & op) const
{
	return (1 << jive_i386_classify_sse);
}

jive_regselect_mask
jive_i386_reg_classifier::classify_float_compare(const jive::flt::compare_op & op) const
{
	return (1 << jive_i386_classify_sse);
}

jive_regselect_mask
jive_i386_reg_classifier::classify_address() const
{
	return (1 << jive_i386_classify_gpr);
}

size_t
jive_i386_reg_classifier::nclasses() const noexcept
{
	return 4;
}

const jive_register_class * const *
jive_i386_reg_classifier::classes() const noexcept
{
	static const jive_register_class * classes [] =  {
		[jive_i386_classify_flags] = &jive_i386_regcls_flags,
		[jive_i386_classify_gpr] = &jive_i386_regcls_gpr,
		[jive_i386_classify_fp] = &jive_i386_regcls_fp,
		[jive_i386_classify_sse] = &jive_i386_regcls_sse,
	};

	return classes;
}
