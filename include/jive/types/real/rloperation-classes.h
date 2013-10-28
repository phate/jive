/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_REAL_RLOPERATION_CLASSES_H
#define JIVE_TYPES_REAL_RLOPERATION_CLASSES_H

#include <jive/vsdg/operators.h>

typedef struct jive_rlbinary_operation_class jive_rlbinary_operation_class;
typedef struct jive_rlcomparison_operation_class jive_rlcomparison_operation_class;
typedef struct jive_rlunary_operation_class jive_rlunary_operation_class;

typedef enum jive_rlop_code {
	jive_rlop_code_invalid = 0,
	jive_rlop_code_sum = 1,
	jive_rlop_code_difference = 2,
	jive_rlop_code_product = 3,
	jive_rlop_code_quotient = 4,
	jive_rlop_code_negate = 5
} jive_rlop_code;

typedef enum jive_rlcmp_code {
	jive_rlcmp_code_invalid = 0,
	jive_rlcmp_code_less = 1,
	jive_rlcmp_code_lesseq = 2,
	jive_rlcmp_code_equal = 3,
	jive_rlcmp_code_notequal = 4,
	jive_rlcmp_code_greatereq = 5,
	jive_rlcmp_code_greater = 6
} jive_rlcmp_code;

struct jive_rlbinary_operation_class {
	jive_binary_operation_class base;
	jive_rlop_code type;
};

struct jive_rlunary_operation_class {
	jive_unary_operation_class base;
	jive_rlop_code type;
};

struct jive_rlcomparison_operation_class {
	jive_binary_operation_class base;
	jive_rlcmp_code type;
};

extern const jive_rlbinary_operation_class JIVE_RLBINARY_NODE_;
#define JIVE_RLBINARY_NODE (JIVE_RLBINARY_NODE_.base.base)

extern const jive_rlunary_operation_class JIVE_RLUNARY_NODE_;
#define JIVE_RLUNARY_NODE (JIVE_RLUNARY_NODE_.base.base)

extern const jive_rlcomparison_operation_class JIVE_RLCOMPARISON_NODE_;
#define JIVE_RLCOMPARISON_NODE (JIVE_RLCOMPARISON_NODE_.base.base)

#endif
