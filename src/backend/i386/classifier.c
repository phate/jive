/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/backend/i386/classifier.h>

#include <jive/backend/i386/registerset.h>
#include <jive/rvsdg/type.h>
#include <jive/types/float/flttype.h>

typedef enum jive_i386_classify_regcls {
	jive_i386_classify_flags = 0,
	jive_i386_classify_gpr = 1,
	jive_i386_classify_fp = 2,
	jive_i386_classify_sse = 3,
} jive_i386_classify_regcls;

namespace jive {
namespace i386 {

static const std::vector<const jive::register_class*>
regclasses({&cc_regcls, &gpr_regcls, &fp_regcls, &xmm_regcls});

register_classifier::~register_classifier() noexcept
{}

jive_regselect_mask
register_classifier::classify_any() const
{
	return (1 << jive_i386_classify_gpr) | (1 << jive_i386_classify_flags);
}

jive_regselect_mask
register_classifier::classify_type(
	const jive::type * type,
	const jive::resource_class * rescls) const
{
	rescls = jive::relax(rescls);

	if (rescls == &gpr_regcls)
		return (1 << jive_i386_classify_gpr);
	else if (rescls == &cc_regcls)
		return (1 << jive_i386_classify_flags);

	auto btype = dynamic_cast<const jive::bits::type*>(type);
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
register_classifier::classify_fixed_unary(
	const jive::bits::unary_op & op) const
{
	return (1 << jive_i386_classify_gpr);
}

jive_regselect_mask
register_classifier::classify_fixed_binary(
	const jive::bits::binary_op & op) const
{
	return (1 << jive_i386_classify_gpr);
}

jive_regselect_mask
register_classifier::classify_fixed_compare(const jive::bits::compare_op & op) const
{
	return (1 << jive_i386_classify_gpr);
}

jive_regselect_mask
register_classifier::classify_float_unary(const jive::flt::unary_op & op) const
{
	return (1 << jive_i386_classify_sse);
}

jive_regselect_mask
register_classifier::classify_float_binary(const jive::flt::binary_op & op) const
{
	return (1 << jive_i386_classify_sse);
}

jive_regselect_mask
register_classifier::classify_float_compare(const jive::flt::compare_op & op) const
{
	return (1 << jive_i386_classify_sse);
}

jive_regselect_mask
register_classifier::classify_address() const
{
	return (1 << jive_i386_classify_gpr);
}

size_t
register_classifier::nclasses() const noexcept
{
	return regclasses.size();
}

const std::vector<const jive::register_class*> &
register_classifier::classes() const noexcept
{
	return regclasses;
}

}}
