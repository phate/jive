/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_REGSELECTOR_H
#define JIVE_ARCH_REGSELECTOR_H

#include <stdint.h>

#include <jive/arch/registers.h>
#include <jive/rvsdg/negotiator.h>
#include <jive/types/bitstring/arithmetic.h>
#include <jive/types/float/arithmetic.h>
#include <jive/types/float/fltoperation-classes.h>

namespace jive {
class output;

namespace bits {
class unary_op;
class binary_op;
class compare_op;
}

class flt_unary_operation;
class flt_binary_operation;
class flt_compare_operation;
};

namespace jive {
namespace base {
	class type;
}

class resource_class;

}

typedef int jive_regselect_index;
typedef uint32_t jive_regselect_mask;

typedef struct jive_reg_classifier jive_reg_classifier;

class jive_reg_classifier {
public:
	virtual ~jive_reg_classifier() noexcept;

	virtual jive_regselect_mask
	classify_any() const = 0;

	virtual jive_regselect_mask
	classify_type(const jive::type * type, const jive::resource_class * rescls) const = 0;

	virtual jive_regselect_mask
	classify_fixed_unary(const jive::bits::unary_op & op) const = 0;

	virtual jive_regselect_mask
	classify_fixed_binary(const jive::bits::binary_op & op) const = 0;

	virtual jive_regselect_mask
	classify_fixed_compare(const jive::bits::compare_op & op) const = 0;

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

	virtual const jive::register_class * const *
	classes() const noexcept = 0;
};

namespace jive {

class register_selector : public negotiator {
public:
	virtual
	~register_selector();

	register_selector(
		jive::graph * graph,
		const jive_reg_classifier * _classifier);

	register_selector(const register_selector &) = delete;

	register_selector(register_selector &&) = delete;

	register_selector &
	operator=(const register_selector &) = delete;

	register_selector &
	operator=(register_selector &&) = delete;

	const jive_reg_classifier * classifier;
};

}

void
jive_regselector_process(jive::register_selector * self);

const jive::register_class *
jive_regselector_map_output(const jive::register_selector * self, jive::simple_output * output);

const jive::register_class *
jive_regselector_map_input(const jive::register_selector * self, jive::simple_input * input);

#endif
