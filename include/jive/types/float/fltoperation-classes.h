/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_FLTOPERATION_CLASSES_H
#define JIVE_TYPES_FLOAT_FLTOPERATION_CLASSES_H

#include <jive/vsdg/operators.h>

namespace jive {

class flt_unary_operation : public unary_operation {
public:
	virtual ~flt_unary_operation() noexcept;
};

class flt_binary_operation : public binary_operation {
public:
	virtual ~flt_binary_operation() noexcept;
};

class flt_compare_operation : public binary_operation {
public:
	virtual ~flt_compare_operation() noexcept;
};

}

typedef struct jive_fltbinary_operation_class jive_fltbinary_operation_class;
typedef struct jive_fltcomparison_operation_class jive_fltcomparison_operation_class;
typedef struct jive_fltunary_operation_class jive_fltunary_operation_class;

typedef enum jive_fltop_code {
	jive_fltop_code_invalid = 0,
	jive_fltop_code_sum = 1,
	jive_fltop_code_difference = 2,
	jive_fltop_code_product = 3,
	jive_fltop_code_quotient = 4,
	jive_fltop_code_negate = 5
} jive_fltop_code;

typedef enum jive_fltcmp_code {
	jive_fltcmp_code_invalid = 0,
	jive_fltcmp_code_less = 1,
	jive_fltcmp_code_lesseq = 2,
	jive_fltcmp_code_equal = 3,
	jive_fltcmp_code_notequal = 4,
	jive_fltcmp_code_greatereq = 5,
	jive_fltcmp_code_greater = 6
} jive_fltcmp_code;

struct jive_fltbinary_operation_class {
	jive_binary_operation_class base;
	jive_fltop_code type;
};

struct jive_fltunary_operation_class {
	jive_unary_operation_class base;
	jive_fltop_code type;
};

struct jive_fltcomparison_operation_class {
	jive_binary_operation_class base;
	jive_fltcmp_code type;
};

extern const jive_fltbinary_operation_class JIVE_FLTBINARY_NODE_;
#define JIVE_FLTBINARY_NODE (JIVE_FLTBINARY_NODE_.base.base)

extern const jive_fltunary_operation_class JIVE_FLTUNARY_NODE_;
#define JIVE_FLTUNARY_NODE (JIVE_FLTUNARY_NODE_.base.base)

extern const jive_fltcomparison_operation_class JIVE_FLTCOMPARISON_NODE_;
#define JIVE_FLTCOMPARISON_NODE (JIVE_FLTCOMPARISON_NODE_.base.base)

#endif
