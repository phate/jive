/*
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_COMPARISON_H
#define JIVE_TYPES_BITSTRING_COMPARISON_H

#include <jive/rvsdg/simple-node.h>
#include <jive/types/bitstring/bitoperation-classes.h>

namespace jive {
namespace bits {

#define DECLARE_BITCOMPARISON_OPERATION(NAME) \
class NAME ## _op final : public compare_op { \
public: \
	virtual \
	~NAME ## _op() noexcept; \
\
	inline \
	NAME ## _op(const jive::bits::type & type) noexcept \
	: compare_op(type) \
	{} \
\
	virtual bool \
	operator==(const operation & other) const noexcept override; \
\
	virtual jive_binary_operation_flags \
	flags() const noexcept override; \
\
	virtual compare_result \
	reduce_constants( \
		const value_repr & arg1, \
		const value_repr & arg2) const override; \
\
	virtual std::string \
	debug_string() const override; \
\
	virtual std::unique_ptr<jive::operation> \
	copy() const override; \
\
	virtual std::unique_ptr<bits::compare_op> \
	create(size_t nbits) const override; \
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

DECLARE_BITCOMPARISON_OPERATION(eq);
DECLARE_BITCOMPARISON_OPERATION(ne);
DECLARE_BITCOMPARISON_OPERATION(sge);
DECLARE_BITCOMPARISON_OPERATION(sgt);
DECLARE_BITCOMPARISON_OPERATION(sle);
DECLARE_BITCOMPARISON_OPERATION(slt);
DECLARE_BITCOMPARISON_OPERATION(uge);
DECLARE_BITCOMPARISON_OPERATION(ugt);
DECLARE_BITCOMPARISON_OPERATION(ule);
DECLARE_BITCOMPARISON_OPERATION(ult);

}
}

#endif
