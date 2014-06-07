/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_REGSELECTOR_H
#define JIVE_ARCH_REGSELECTOR_H

#include <stdint.h>

#include <jive/arch/registers.h>
#include <jive/types/bitstring/arithmetic.h>
#include <jive/types/float/arithmetic.h>
#include <jive/types/float/fltoperation-classes.h>
#include <jive/vsdg/negotiator.h>

struct jive_graph;
struct jive_output;
struct jive_resource_class;

namespace jive {
namespace base {
	class type;
}
	class input;
}

typedef int jive_regselect_index;
typedef uint32_t jive_regselect_mask;

typedef struct jive_reg_classifier jive_reg_classifier;

struct jive_reg_classifier {
	jive_regselect_mask any;
	jive_regselect_mask (*classify_type)(const struct jive::base::type * type,
		const struct jive_resource_class * rescls);
	jive_regselect_mask (*classify_fixed_arithmetic)(jive_bitop_code op, size_t nbits);
	jive_regselect_mask (*classify_float_arithmetic)(jive_fltop_code op);
	jive_regselect_mask (*classify_fixed_compare)(jive_bitcmp_code op, size_t nbits);
	jive_regselect_mask (*classify_float_compare)(jive_fltcmp_code op);
	jive_regselect_mask (*classify_address)(void);
	
	size_t nclasses;
	const jive_register_class * const * classes;
};

JIVE_EXPORTED_INLINE jive_regselect_mask
jive_reg_classifier_classify_type(const jive_reg_classifier * self,
	const struct jive::base::type * type, const struct jive_resource_class * rescls)
{
	return self->classify_type(type, rescls);
}

JIVE_EXPORTED_INLINE jive_regselect_mask
jive_reg_classifier_classify_fixed_arithmetic(const jive_reg_classifier * self,
	jive_bitop_code op, size_t nbits)
{
	return self->classify_fixed_arithmetic(op, nbits);
}

JIVE_EXPORTED_INLINE jive_regselect_mask
jive_reg_classifier_classify_float_arithmetic(const jive_reg_classifier * self,
	jive_fltop_code op)
{
	return self->classify_float_arithmetic(op);
}

JIVE_EXPORTED_INLINE jive_regselect_mask
jive_reg_classifier_classify_fixed_compare(const jive_reg_classifier * self,
	jive_bitcmp_code op, size_t nbits)
{
	return self->classify_fixed_compare(op, nbits);
}

JIVE_EXPORTED_INLINE jive_regselect_mask
jive_reg_classifier_classify_float_compare(const jive_reg_classifier * self,
	jive_fltcmp_code op)
{
	return self->classify_float_compare(op);
}

JIVE_EXPORTED_INLINE jive_regselect_mask
jive_reg_classifier_classify_address(const jive_reg_classifier * self)
{
	return self->classify_address();
}

typedef struct jive_regselector jive_regselector;

struct jive_regselector {
	jive_negotiator base;
	const jive_reg_classifier * classifier;
};

void
jive_regselector_init(jive_regselector * self, struct jive_graph * graph,
	const jive_reg_classifier * classifier);

void
jive_regselector_process(jive_regselector * self);

const jive_register_class *
jive_regselector_map_output(const jive_regselector * self, struct jive_output * output);

const jive_register_class *
jive_regselector_map_input(const jive_regselector * self, jive::input * input);

void
jive_regselector_fini(jive_regselector * self);

#endif
