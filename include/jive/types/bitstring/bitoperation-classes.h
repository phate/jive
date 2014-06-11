/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_BITOPERATION_CLASSES_H
#define JIVE_TYPES_BITSTRING_BITOPERATION_CLASSES_H

#include <jive/types/bitstring/type.h>
#include <jive/types/bitstring/value-representation.h>
#include <jive/vsdg/operators.h>

namespace jive {

/* Represents a unary operation on a bitstring of a specific width,
 * produces another bitstring of the same width. */
class bits_unary_operation : public unary_operation {
public:
	virtual ~bits_unary_operation() noexcept;

	inline bits_unary_operation(const jive::bits::type & type) noexcept : type_(type) {}

	/* type signature methods */
	virtual size_t
	narguments() const noexcept override;

	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	inline const jive::bits::type & type() const noexcept { return type_; }

	/* reduction methods */
	virtual jive_unop_reduction_path_t
	can_reduce_operand(
		const jive::output * arg) const noexcept override;

	virtual jive::output *
	reduce_operand(
		jive_unop_reduction_path_t path,
		jive::output * arg) const override;

	virtual bitstring::value_repr
	reduce_constant(
		const bitstring::value_repr & arg) const = 0;

private:
	jive::bits::type type_;
};

/* Represents a binary operation (possibly normalized n-ary if associative)
 * on a bitstring of a specific width, produces another bitstring of the
 * same width. */
class bits_binary_operation : public binary_operation {
public:
	virtual ~bits_binary_operation() noexcept;

	inline bits_binary_operation(const jive::bits::type & type, size_t arity = 2) noexcept
		: type_(type)
		, arity_(arity)
	{}

	/* type signature methods */
	virtual size_t
	narguments() const noexcept override;

	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	/* reduction methods */
	virtual jive_binop_reduction_path_t
	can_reduce_operand_pair(
		const jive::output * arg1,
		const jive::output * arg2) const noexcept override;

	virtual jive::output *
	reduce_operand_pair(
		jive_binop_reduction_path_t path,
		jive::output * arg1,
		jive::output * arg2) const override;

	virtual bitstring::value_repr
	reduce_constants(
		const bitstring::value_repr & arg1,
		const bitstring::value_repr & arg2) const = 0;


	inline const jive::bits::type & type() const noexcept { return type_; }

	inline size_t
	arity() const noexcept { return arity_; }

private:
	jive::bits::type type_;
	size_t arity_;
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
