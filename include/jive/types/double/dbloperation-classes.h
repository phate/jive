/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_DOUBLE_DBLOPERATION_CLASSES_H
#define JIVE_TYPES_DOUBLE_DBLOPERATION_CLASSES_H

#include <jive/vsdg/operators.h>

typedef struct jive_dblbinary_operation_class jive_dblbinary_operation_class;
typedef struct jive_dblcomparison_operation_class jive_dblcomparison_operation_class;
typedef struct jive_dblunary_operation_class jive_dblunary_operation_class;

typedef enum jive_dblop_code {
	jive_dblop_code_invalid = 0,
	jive_dblop_code_sum = 1,
	jive_dblop_code_difference = 2,
	jive_dblop_code_product = 3,
	jive_dblop_code_quotient = 4,
	jive_dblop_code_negate = 5
} jive_dblop_code;

typedef enum jive_dblcmp_code {
	jive_dblcmp_code_invalid = 0,
	jive_dblcmp_code_less = 1,
	jive_dblcmp_code_lesseq = 2,
	jive_dblcmp_code_equal = 3,
	jive_dblcmp_code_notequal = 4,
	jive_dblcmp_code_greatereq = 5,
	jive_dblcmp_code_greater = 6
} jive_dblcmp_code;

struct jive_dblbinary_operation_class {
	jive_binary_operation_class base;
	jive_dblop_code type;
};

struct jive_dblunary_operation_class {
	jive_unary_operation_class base;
	jive_dblop_code type;
};

struct jive_dblcomparison_operation_class {
	jive_binary_operation_class base;
	jive_dblcmp_code type;
};

extern const jive_dblbinary_operation_class JIVE_DBLBINARY_NODE_;
#define JIVE_DBLBINARY_NODE (JIVE_DBLBINARY_NODE_.base.base)

extern const jive_dblunary_operation_class JIVE_DBLUNARY_NODE_;
#define JIVE_DBLUNARY_NODE (JIVE_DBLUNARY_NODE_.base.base)

extern const jive_dblcomparison_operation_class JIVE_DBLCOMPARISON_NODE_;
#define JIVE_DBLCOMPARISON_NODE (JIVE_DBLCOMPARISON_NODE_.base.base)

#endif
