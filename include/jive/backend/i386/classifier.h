/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_BACKEND_I386_CLASSIFIER_H
#define JIVE_BACKEND_I386_CLASSIFIER_H

#include <jive/arch/regselector.h>

class jive_i386_reg_classifier final : public jive_reg_classifier {
public:
	virtual ~jive_i386_reg_classifier() noexcept;

	virtual jive_regselect_mask
	classify_any() const override;

	virtual jive_regselect_mask
	classify_type(const jive::base::type * type, const jive_resource_class * rescls) const override;

	virtual jive_regselect_mask
	classify_fixed_unary(const jive::bits_unary_operation & op) const override;

	virtual jive_regselect_mask
	classify_fixed_binary(const jive::bits_binary_operation & op) const override;

	virtual jive_regselect_mask
	classify_fixed_compare(const jive::bits_compare_operation & op) const override;

	virtual jive_regselect_mask
	classify_float_unary(const jive::flt_unary_operation & op) const override;

	virtual jive_regselect_mask
	classify_float_binary(const jive::flt_binary_operation & op) const override;

	virtual jive_regselect_mask
	classify_float_compare(const jive::flt_compare_operation & op) const override;

	virtual jive_regselect_mask
	classify_address() const override;

	virtual size_t
	nclasses() const noexcept override;

	virtual const jive_register_class * const *
	classes() const noexcept override;
};

#endif
