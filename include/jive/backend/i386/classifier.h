/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
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
	classify_type(const jive::base::type * type, const jive::resource_class * rescls) const override;

	virtual jive_regselect_mask
	classify_fixed_unary(const jive::bits::unary_op & op) const override;

	virtual jive_regselect_mask
	classify_fixed_binary(const jive::bits::binary_op & op) const override;

	virtual jive_regselect_mask
	classify_fixed_compare(const jive::bits::compare_op & op) const override;

	virtual jive_regselect_mask
	classify_float_unary(const jive::flt::unary_op & op) const override;

	virtual jive_regselect_mask
	classify_float_binary(const jive::flt::binary_op & op) const override;

	virtual jive_regselect_mask
	classify_float_compare(const jive::flt::compare_op & op) const override;

	virtual jive_regselect_mask
	classify_address() const override;

	virtual size_t
	nclasses() const noexcept override;

	virtual const jive_register_class * const *
	classes() const noexcept override;
};

#endif
