/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_REGSELECTOR_H
#define JIVE_ARCH_REGSELECTOR_H

#include <jive/arch/registers.h>
#include <jive/types/bitstring/arithmetic.h>
#include <jive/types/float/arithmetic.h>
#include <jive/types/float/fltoperation-classes.h>

namespace jive {
namespace base {
	class type;
}

class resource_class;

}

typedef int jive_regselect_index;
typedef uint32_t jive_regselect_mask;

namespace jive {

class bitunary_op;
class bitbinary_op;
class bitcompare_op;
class output;

class flt_unary_operation;
class flt_binary_operation;
class flt_compare_operation;

class register_classifier {
public:
	virtual ~register_classifier() noexcept;

	virtual jive_regselect_mask
	classify_any() const = 0;

	virtual jive_regselect_mask
	classify_type(const jive::type * type, const jive::resource_class * rescls) const = 0;

	virtual jive_regselect_mask
	classify_fixed_unary(const jive::bitunary_op & op) const = 0;

	virtual jive_regselect_mask
	classify_fixed_binary(const jive::bitbinary_op & op) const = 0;

	virtual jive_regselect_mask
	classify_fixed_compare(const jive::bitcompare_op & op) const = 0;

	virtual jive_regselect_mask
	classify_float_unary(const jive::flt::unary_op & op) const = 0;

	virtual jive_regselect_mask
	classify_float_binary(const jive::flt::binary_op & op) const = 0;

	virtual jive_regselect_mask
	classify_float_compare(const jive::flt::compare_op & op) const = 0;

	virtual jive_regselect_mask
	classify_address() const = 0;

	virtual size_t
	nclasses() const noexcept = 0;

	virtual const std::vector<const jive::register_class*> &
	classes() const noexcept = 0;
};

}

#endif
