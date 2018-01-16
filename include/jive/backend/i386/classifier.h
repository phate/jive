/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_BACKEND_I386_CLASSIFIER_H
#define JIVE_BACKEND_I386_CLASSIFIER_H

#include <jive/arch/regselector.h>

namespace jive {
namespace i386 {

class register_classifier final : public jive::register_classifier {
public:
	virtual
	~register_classifier() noexcept;

	virtual jive_regselect_mask
	classify_any() const override;

	virtual jive_regselect_mask
	classify_type(const jive::type * type, const jive::resource_class * rescls) const override;

	virtual jive_regselect_mask
	classify_fixed_unary(const jive::bitunary_op & op) const override;

	virtual jive_regselect_mask
	classify_fixed_binary(const jive::bitbinary_op & op) const override;

	virtual jive_regselect_mask
	classify_fixed_compare(const jive::bitcompare_op & op) const override;

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

	virtual const std::vector<const jive::register_class*> &
	classes() const noexcept override;

	static inline const register_classifier *
	get()
	{
		static const register_classifier classifier;
		return &classifier;
	}
};

}}

#endif
