/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_OPERATORS_UNARY_H
#define JIVE_VSDG_OPERATORS_UNARY_H

#include <jive/common.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators/base.h>

typedef size_t jive_unop_reduction_path_t;

namespace jive {

/**
	\brief Unary operator
	
	Operator taking a single argument.
*/
class unary_operation : public operation {
public:
	virtual ~unary_operation() noexcept;

	virtual size_t
	narguments() const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual jive_unop_reduction_path_t
	can_reduce_operand(
		const jive::output * arg) const noexcept;

	virtual jive::output *
	reduce_operand(
		jive_unop_reduction_path_t path,
		jive::output * arg) const;
};

}

typedef struct jive_unary_operation_class jive_unary_operation_class;

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

/* node class */

struct jive_unary_operation_class {
	jive_node_class base;
	
	/* algebraic properties */
	const struct jive_binary_operation_class * const * single_apply_over;
	const struct jive_binary_operation_class * const * multi_apply_over;
	
	/* class method */
	jive_unop_reduction_path_t (*can_reduce_operand)(const jive_node_class * cls,
		const jive_node_attrs * attrs, const jive::output * operand);
	
	/* class method */
	jive::output * (*reduce_operand)(jive_unop_reduction_path_t path, const jive_node_class * cls,
		const jive_node_attrs * attrs, jive::output * operand);
};

extern const jive_unary_operation_class JIVE_UNARY_OPERATION_;
#define JIVE_UNARY_OPERATION (JIVE_UNARY_OPERATION_.base)

JIVE_EXPORTED_INLINE jive::output *
jive_unary_operation_create_normalized(const jive_node_class * class_,
	struct jive_graph * graph, const jive_node_attrs * attrs, jive::output * operand)
{
	jive::output * result;
	jive_node_create_normalized(class_, graph, attrs, 1, &operand, &result);
	return result;
}

JIVE_EXPORTED_INLINE jive::output *
jive_unary_operation_create_normalized(const jive_unary_operation_class * class_,
	struct jive_graph * graph, const jive_node_attrs * attrs, jive::output * operand)
{
	return jive_unary_operation_create_normalized(&class_->base, graph, attrs, operand);
}

/* node class inheritable methods */

jive_node_normal_form *
jive_unary_operation_get_default_normal_form_(
	const jive_node_class * cls,
	jive_node_normal_form * parent,
	struct jive_graph * graph);

jive_unop_reduction_path_t
jive_unary_operation_can_reduce_operand_(const jive_node_class * cls,
	const jive_node_attrs * attrs,
	const jive::output * operand);

jive::output *
jive_unary_operation_reduce_operand_(jive_unop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs, jive::output * operand);

/* normal form class */

struct jive_unary_operation_normal_form_class {
	jive_node_normal_form_class base;
	void (*set_reducible)(jive_unary_operation_normal_form * self, bool enable);
	jive::output * (*normalized_create)(
		const jive_unary_operation_normal_form * self,
		struct jive_region * region,
		const jive_node_attrs * attrs,
		jive::output * operand);
};

extern const jive_unary_operation_normal_form_class JIVE_UNARY_OPERATION_NORMAL_FORM_;
#define JIVE_UNARY_OPERATION_NORMAL_FORM (JIVE_UNARY_OPERATION_NORMAL_FORM_.base)

struct jive_unary_operation_normal_form {
	jive_node_normal_form base;
	bool enable_reducible;
};

JIVE_EXPORTED_INLINE jive_unary_operation_normal_form *
jive_unary_operation_normal_form_cast(jive_node_normal_form * self)
{
	const jive_node_normal_form_class * cls = self->class_;
	while (cls) {
		if (cls == &JIVE_UNARY_OPERATION_NORMAL_FORM)
			return (jive_unary_operation_normal_form *)self;
		cls = cls->parent;
	}
	return 0;
}

JIVE_EXPORTED_INLINE jive::output *
jive_unary_operation_normalized_create(
	const jive_unary_operation_normal_form * self,
	struct jive_region * region,
	const jive_node_attrs * attrs,
	jive::output * operand)
{
	const jive_unary_operation_normal_form_class * cls;
	cls = (const jive_unary_operation_normal_form_class *) self->base.class_;
	
	return cls->normalized_create(self, region, attrs, operand);
}

JIVE_EXPORTED_INLINE jive_unop_reduction_path_t
jive_unary_operation_can_reduce_operand(const jive_unary_operation_normal_form * self,
	const jive_node_attrs * attrs, const jive::output * operand)
{
	const jive_unary_operation_class * cls =
		(const jive_unary_operation_class *)self->base.node_class;
	if (cls->can_reduce_operand) {
		return cls->can_reduce_operand(self->base.node_class, attrs, operand);
	} else {
		const jive::unary_operation & op =
			static_cast<const jive::unary_operation &>(*attrs);
		return op.can_reduce_operand(operand);
	}
}

JIVE_EXPORTED_INLINE jive::output *
jive_unary_operation_reduce_operand(jive_unop_reduction_path_t path,
	const jive_unary_operation_normal_form * self, const jive_node_attrs * attrs,
	jive::output * operand)
{
	const jive_unary_operation_class * cls =
		(const jive_unary_operation_class *)self->base.node_class;
	if (cls->reduce_operand) {
		return cls->reduce_operand(path, self->base.node_class, attrs, operand);
	} else {
		const jive::unary_operation & op =
			static_cast<const jive::unary_operation &>(*attrs);
		return op.reduce_operand(path, operand);
	}
}

JIVE_EXPORTED_INLINE void
jive_unary_operation_set_reducible(jive_unary_operation_normal_form * self, bool reducible)
{
	const jive_unary_operation_normal_form_class * cls;
	cls = (const jive_unary_operation_normal_form_class *) self->base.class_;
	cls->set_reducible(self, reducible);
}

/* normal form class inheritable methods */

bool
jive_unary_operation_normalize_node_(const jive_node_normal_form * self, jive_node * node);

bool
jive_unary_operation_operands_are_normalized_(
	const jive_node_normal_form * self,
	size_t noperands,
	jive::output * const operands[],
	const jive_node_attrs * attrs);

jive::output *
jive_unary_operation_normalized_create_(
	const jive_unary_operation_normal_form * self,
	jive_region * region,
	const jive_node_attrs * attrs,
	jive::output * operand);

void
jive_unary_operation_set_reducible_(jive_unary_operation_normal_form * self_, bool enable);

#endif
