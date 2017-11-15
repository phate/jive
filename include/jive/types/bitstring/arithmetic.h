/*
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_H

#include <jive/types/bitstring/bitoperation-classes.h>
#include <jive/vsdg/simple-node.h>

namespace jive {
namespace bits {

#define DECLARE_BITUNARY_OPERATION(NAME) \
class NAME ## _op final : public unary_op { \
public: \
	virtual \
	~NAME ## _op() noexcept; \
\
	inline \
	NAME ## _op(const jive::bits::type & type) noexcept \
	: unary_op(type) \
	{} \
\
	virtual bool \
	operator==(const operation & other) const noexcept override; \
\
	virtual value_repr \
	reduce_constant(const value_repr & arg) const override; \
\
	virtual std::string \
	debug_string() const override; \
\
	virtual std::unique_ptr<jive::operation> \
	copy() const override; \
}; \
\
static inline jive::output * \
create_##NAME(size_t nbits, jive::output * op) \
{ \
	return create_normalized(op->region(), NAME ## _op(nbits), {op})[0]; \
} \
\
static inline bool \
is_ ## NAME ## _node(const jive::node * node) noexcept \
{ \
	return is_opnode<NAME ## _op>(node); \
} \

#define DECLARE_BITBINARY_OPERATION(NAME) \
class NAME ## _op final : public binary_op { \
public: \
	virtual \
	~NAME ## _op() noexcept; \
\
	inline \
	NAME ## _op(const jive::bits::type & type) noexcept \
	: binary_op(type) \
	{} \
\
	virtual bool \
	operator==(const operation & other) const noexcept override; \
\
	virtual jive_binary_operation_flags \
	flags() const noexcept override; \
\
	virtual value_repr \
	reduce_constants( \
		const value_repr & arg1, \
		const value_repr & arg2) const override; \
\
	virtual std::string \
	debug_string() const override; \
\
	virtual std::unique_ptr<jive::operation> \
	copy() const override; \
}; \
\
static inline jive::output * \
create_##NAME(size_t nbits, jive::output * op1, jive::output * op2) \
{ \
	return create_normalized(op1->region(), NAME ## _op(nbits), {op1, op2})[0]; \
} \
\
static inline bool \
is_ ## NAME ## _node(const jive::node * node) noexcept \
{ \
	return is_opnode<NAME ## _op>(node); \
} \

DECLARE_BITUNARY_OPERATION(neg);
DECLARE_BITUNARY_OPERATION(not);

DECLARE_BITBINARY_OPERATION(add);
DECLARE_BITBINARY_OPERATION(and);
DECLARE_BITBINARY_OPERATION(ashr);
DECLARE_BITBINARY_OPERATION(mul);
DECLARE_BITBINARY_OPERATION(or);
DECLARE_BITBINARY_OPERATION(sdiv);
DECLARE_BITBINARY_OPERATION(shl);
DECLARE_BITBINARY_OPERATION(shr);
DECLARE_BITBINARY_OPERATION(smod);
DECLARE_BITBINARY_OPERATION(smulh);
DECLARE_BITBINARY_OPERATION(sub);
DECLARE_BITBINARY_OPERATION(udiv);
DECLARE_BITBINARY_OPERATION(umod);
DECLARE_BITBINARY_OPERATION(umulh);
DECLARE_BITBINARY_OPERATION(xor);

}
}

#endif
