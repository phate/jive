/*
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_BITOPERATION_CLASSES_H
#define JIVE_TYPES_BITSTRING_BITOPERATION_CLASSES_H

#include <jive/types/bitstring/type.h>
#include <jive/vsdg/operators.h>

namespace jive {

class bits_unary_operation : public unary_operation {
public:
	virtual ~bits_unary_operation() noexcept;
};

class bits_binary_operation : public binary_operation {
public:
	virtual ~bits_binary_operation() noexcept;
};

class bits_compare_operation : public binary_operation {
public:
	virtual ~bits_compare_operation() noexcept;
};

}

typedef struct jive_bitbinary_operation_class jive_bitbinary_operation_class;
typedef struct jive_bitcomparison_operation_class jive_bitcomparison_operation_class;
typedef struct jive_bitunary_operation_class jive_bitunary_operation_class;

typedef enum jive_bitop_code {
	jive_bitop_code_invalid = 0,
	jive_bitop_code_and = 1,
	jive_bitop_code_or = 2,
	jive_bitop_code_xor = 3,
	jive_bitop_code_sum = 4,
	jive_bitop_code_difference = 5,
	jive_bitop_code_product = 6,
	jive_bitop_code_uhiproduct = 7,
	jive_bitop_code_shiproduct = 8,
	jive_bitop_code_uquotient = 9,
	jive_bitop_code_squotient = 10,
	jive_bitop_code_umod = 11,
	jive_bitop_code_smod = 12,
	jive_bitop_code_shl = 13,
	jive_bitop_code_shr = 14,
	jive_bitop_code_ashr = 15,
	jive_bitop_code_negate = 16,
	jive_bitop_code_not = 17
} jive_bitop_code;

typedef enum jive_bitcmp_code {
	jive_bitcmp_code_invalid = 0,
	jive_bitcmp_code_equal = 1,
	jive_bitcmp_code_notequal = 2,
	jive_bitcmp_code_sless = 3,
	jive_bitcmp_code_uless = 4,
	jive_bitcmp_code_slesseq = 5,
	jive_bitcmp_code_ulesseq = 6,
	jive_bitcmp_code_sgreater = 7,
	jive_bitcmp_code_ugreater = 8,
	jive_bitcmp_code_sgreatereq = 9,
	jive_bitcmp_code_ugreatereq = 10
} jive_bitcmp_code;

struct jive_bitbinary_operation_class {
	jive_binary_operation_class base;
	jive_bitop_code type;
};

struct jive_bitunary_operation_class {
	jive_unary_operation_class base;
	jive_bitop_code type;
};

struct jive_bitcomparison_operation_class {
	jive_binary_operation_class base;
	jive_bitcmp_code type;
	char (*compare_constants)(const char * c1, const char * c2, size_t nbits);
};

extern const jive_bitbinary_operation_class JIVE_BITBINARY_NODE_;
#define JIVE_BITBINARY_NODE (JIVE_BITBINARY_NODE_.base.base)

extern const jive_bitunary_operation_class JIVE_BITUNARY_NODE_;
#define JIVE_BITUNARY_NODE (JIVE_BITUNARY_NODE_.base.base)

extern const jive_bitcomparison_operation_class JIVE_BITCOMPARISON_NODE_;
#define JIVE_BITCOMPARISON_NODE (JIVE_BITCOMPARISON_NODE_.base.base)

#endif
