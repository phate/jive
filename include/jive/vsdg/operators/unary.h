/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_OPERATORS_UNARY_H
#define JIVE_VSDG_OPERATORS_UNARY_H

#include <jive/common.h>
#include <jive/vsdg/node-normal-form.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators/base.h>

typedef size_t jive_unop_reduction_path_t;

namespace jive {
namespace base {

/**
	\brief Unary operator
	
	Operator taking a single argument.
*/
class unary_op : public operation {
public:
	virtual ~unary_op() noexcept;

	virtual size_t
	narguments() const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual jive_unop_reduction_path_t
	can_reduce_operand(
		const jive::oport * arg) const noexcept = 0;

	virtual jive::oport *
	reduce_operand(
		jive_unop_reduction_path_t path,
		jive::oport * arg) const = 0;
};

}
}

typedef struct jive_unary_operation_normal_form jive_unary_operation_normal_form;
typedef struct jive_unary_operation_normal_form_class jive_unary_operation_normal_form_class;

static const jive_unop_reduction_path_t jive_unop_reduction_none = 0;
/* operation is applied to constant, compute immediately */
static const jive_unop_reduction_path_t jive_unop_reduction_constant = 1;
/* operation does not change input operand */
static const jive_unop_reduction_path_t jive_unop_reduction_idempotent = 2;
/* operation is applied on inverse operation, can eliminate */
static const jive_unop_reduction_path_t jive_unop_reduction_inverse = 4;
/* operation "supersedes" immediately preceding operation */
static const jive_unop_reduction_path_t jive_unop_reduction_narrow = 5;
/* operation can be distributed into operands of preceding operation */
static const jive_unop_reduction_path_t jive_unop_reduction_distribute = 6;

#endif
