/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_INTEGRAL_ITGOPERATION_CLASSES_H
#define JIVE_TYPES_INTEGRAL_ITGOPERATION_CLASSES_H

#include <jive/vsdg/operators/binary.h>
#include <jive/vsdg/operators/unary.h>

typedef struct jive_itgbinary_operation_class jive_itgbinary_operation_class;
typedef struct jive_itgcomparison_operation_class jive_itgcomparison_operation_class;
typedef struct jive_itgunary_operation_class jive_itgunary_operation_class;

typedef enum jive_itgop_code {
	jive_itgop_code_invalid = 0,
	jive_itgop_code_and = 1,
	jive_itgop_code_or = 2,
	jive_itgop_code_xor = 3,
	jive_itgop_code_sum = 4,
	jive_itgop_code_difference = 5,
	jive_itgop_code_product = 6,
	jive_itgop_code_quotient = 7,
	jive_itgop_code_modulo = 8,
	jive_itgop_code_not = 9,
	jive_itgop_code_negate = 10
} jive_itgop_code;

typedef enum jive_itgcmp_code {
	jive_itgcmp_code_invalid = 0,
	jive_itgcmp_code_less = 1,
	jive_itgcmp_code_lesseq = 2,
	jive_itgcmp_code_equal = 3,
	jive_itgcmp_code_notequal = 4,
	jive_itgcmp_code_greater = 5,
	jive_itgcmp_code_greatereq = 6
} jive_itgcmp_code;

struct jive_itgunary_operation_class {
	jive_unary_operation_class base;
	jive_itgop_code type;
};

struct jive_itgbinary_operation_class {
	jive_binary_operation_class base;
	jive_itgop_code type;
};

struct jive_itgcomparison_operation_class {
	jive_binary_operation_class base;
	jive_itgcmp_code type;
};

extern const jive_itgunary_operation_class JIVE_ITGUNARY_NODE_;
#define JIVE_ITGUNARY_NODE (JIVE_ITGUNARY_NODE_.base.base)

extern const jive_itgbinary_operation_class JIVE_ITGBINARY_NODE_;
#define JIVE_ITGBINARY_NODE (JIVE_ITGBINARY_NODE_.base.base)

extern const jive_itgcomparison_operation_class JIVE_ITGCOMPARISON_NODE_;
#define JIVE_ITGCOMPARISON_NODE (JIVE_ITGCOMPARISON_NODE_.base.base)

#endif
