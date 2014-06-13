/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_OPERATORS_BINARY_H
#define JIVE_VSDG_OPERATORS_BINARY_H

#include <jive/common.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators/base.h>

typedef size_t jive_binop_reduction_path_t;

enum jive_binary_operation_flags {
	jive_binary_operation_none = 0,
	jive_binary_operation_associative = 1,
	jive_binary_operation_commutative = 2
};

namespace jive {

/**
	\brief Binary operator
	
	Operator taking two arguments (with well-defined reduction for more
	operands if operator is associative).
*/
class binary_operation : public operation {
public:
	virtual ~binary_operation() noexcept;

	virtual jive_binop_reduction_path_t
	can_reduce_operand_pair(
		const jive::output * op1,
		const jive::output * op2) const noexcept;

	virtual jive::output *
	reduce_operand_pair(
		jive_binop_reduction_path_t path,
		jive::output * op1,
		jive::output * op2) const;

	virtual jive_binary_operation_flags
	flags() const noexcept;

	inline bool
	is_associative() const noexcept
	{
		return flags() & jive_binary_operation_associative;
	}

	inline bool
	is_commutative() const noexcept
	{
		return flags() & jive_binary_operation_commutative;
	}
};

}

struct jive_region;

typedef struct jive_binary_operation_normal_form jive_binary_operation_normal_form;
typedef struct jive_binary_operation_normal_form_class jive_binary_operation_normal_form_class;

static const jive_binop_reduction_path_t jive_binop_reduction_none = 0;
/* both operands are constants */
static const jive_binop_reduction_path_t jive_binop_reduction_constants = 1;
/* can merge both operands into single (using some "simpler" operator) */
static const jive_binop_reduction_path_t jive_binop_reduction_merge = 2;
/* part of left operand can be folded into right */
static const jive_binop_reduction_path_t jive_binop_reduction_lfold = 3;
/* part of right operand can be folded into left */
static const jive_binop_reduction_path_t jive_binop_reduction_rfold = 4;
/* left operand is neutral element */
static const jive_binop_reduction_path_t jive_binop_reduction_lneutral = 5;
/* right operand is neutral element */
static const jive_binop_reduction_path_t jive_binop_reduction_rneutral = 6;
/* both operands have common form which can be factored over op */
static const jive_binop_reduction_path_t jive_binop_reduction_factor = 7;

/* node class */

extern const jive_node_class JIVE_BINARY_OPERATION;

JIVE_EXPORTED_INLINE jive::output *
jive_binary_operation_create_normalized(const jive_node_class * class_,
	struct jive_graph * graph, const jive_node_attrs * attrs, size_t noperands,
	jive::output * const operands[])
{
	jive::output * result;
	jive_node_create_normalized(class_, graph, attrs, noperands, operands, &result);
	return result;
}

/* node class inheritable methods */

jive_node_normal_form *
jive_binary_operation_get_default_normal_form_(
	const jive_node_class * cls,
	jive_node_normal_form * parent,
	jive_graph * graph);

/* normal form class */

struct jive_binary_operation_normal_form_class {
	jive_node_normal_form_class base;
	void (*set_reducible)(jive_binary_operation_normal_form * self, bool enable);
	void (*set_flatten)(jive_binary_operation_normal_form * self, bool enable);
	void (*set_reorder)(jive_binary_operation_normal_form * self, bool enable);
	void (*set_distribute)(jive_binary_operation_normal_form * self, bool enable);
	void (*set_factorize)(jive_binary_operation_normal_form * self, bool enable);
	jive::output * (*normalized_create)(
		const jive_binary_operation_normal_form * self,
		struct jive_region * region,
		const jive_node_attrs * attrs,
		size_t noperands,
		jive::output * const operands[]);
};

extern const jive_binary_operation_normal_form_class JIVE_BINARY_OPERATION_NORMAL_FORM_;
#define JIVE_BINARY_OPERATION_NORMAL_FORM (JIVE_BINARY_OPERATION_NORMAL_FORM_.base)

struct jive_binary_operation_normal_form {
	jive_node_normal_form base;
	bool enable_reducible;
	bool enable_flatten;
	bool enable_reorder;
	bool enable_distribute;
	bool enable_factorize;
};

JIVE_EXPORTED_INLINE jive_binary_operation_normal_form *
jive_binary_operation_normal_form_cast(jive_node_normal_form * self)
{
	const jive_node_normal_form_class * cls = self->class_;
	while (cls) {
		if (cls == &JIVE_BINARY_OPERATION_NORMAL_FORM)
			return (jive_binary_operation_normal_form *) self;
		cls = cls->parent;
	}
	return 0;
}

JIVE_EXPORTED_INLINE void
jive_binary_operation_set_reducible(jive_binary_operation_normal_form * self, bool reducible)
{
	const jive_binary_operation_normal_form_class * cls;
	cls = (const jive_binary_operation_normal_form_class *) self->base.class_;
	cls->set_reducible(self, reducible);
}

JIVE_EXPORTED_INLINE void
jive_binary_operation_set_flatten(jive_binary_operation_normal_form * self, bool flatten)
{
	const jive_binary_operation_normal_form_class * cls;
	cls = (const jive_binary_operation_normal_form_class *) self->base.class_;
	cls->set_flatten(self, flatten);
}

JIVE_EXPORTED_INLINE void
jive_binary_operation_set_reorder(jive_binary_operation_normal_form * self, bool reorder)
{
	const jive_binary_operation_normal_form_class * cls;
	cls = (const jive_binary_operation_normal_form_class *) self->base.class_;
	cls->set_reorder(self, reorder);
}

JIVE_EXPORTED_INLINE void
jive_binary_operation_set_distribute(jive_binary_operation_normal_form * self, bool distribute)
{
	const jive_binary_operation_normal_form_class * cls;
	cls = (const jive_binary_operation_normal_form_class *) self->base.class_;
	cls->set_distribute(self, distribute);
}

JIVE_EXPORTED_INLINE void
jive_binary_operation_set_factorize(jive_binary_operation_normal_form * self, bool factorize)
{
	const jive_binary_operation_normal_form_class * cls;
	cls = (const jive_binary_operation_normal_form_class *) self->base.class_;
	cls->set_factorize(self, factorize);
}

/* normal form class inheritable methods */

bool
jive_binary_operation_normalize_node_(const jive_node_normal_form * self, jive_node * node);

bool
jive_binary_operation_operands_are_normalized_(
	const jive_node_normal_form * self,
	size_t noperands,
	jive::output * const operands[],
	const jive_node_attrs * attrs);

void
jive_binary_operation_set_reducible_(jive_binary_operation_normal_form * self, bool enable);

void
jive_binary_operation_set_flatten_(jive_binary_operation_normal_form * self, bool enable);

void
jive_binary_operation_set_reorder_(jive_binary_operation_normal_form * self, bool enable);

void
jive_binary_operation_set_distribute_(jive_binary_operation_normal_form * self, bool enable);

void
jive_binary_operation_set_factorize_(jive_binary_operation_normal_form * self, bool enable);

jive::output *
jive_binary_operation_normalized_create_(
	const jive_binary_operation_normal_form * self,
	struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive::output * const operands[]);

#endif
