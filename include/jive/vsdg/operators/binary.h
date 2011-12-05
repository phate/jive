#ifndef JIVE_VSDG_OPERATORS_BINARY_H
#define JIVE_VSDG_OPERATORS_BINARY_H

#include <jive/common.h>
#include <jive/vsdg/node.h>

struct jive_region;

struct jive_unary_operation_class;

typedef struct jive_binary_operation_class jive_binary_operation_class;

typedef struct jive_binary_operation_normal_form_class jive_binary_operation_normal_form_class;
typedef struct jive_binary_operation_normal_form jive_binary_operation_normal_form;

/* node class */

typedef enum jive_binary_operation_flags {
	jive_binary_operation_none = 0,
	jive_binary_operation_associative = 1,
	jive_binary_operation_commutative = 2
} jive_binary_operation_flags;

struct jive_binary_operation_class {
	jive_node_class base;
	
	/* algebraic properties */
	jive_binary_operation_flags flags;
	const struct jive_unary_operation_class * const * single_apply_under;
	const struct jive_unary_operation_class * const * multi_apply_under;
	const jive_binary_operation_class * const * distributive_over;
	const jive_binary_operation_class * const * distributive_under;
	
	bool (*can_reduce_operand_pair)(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * op1, jive_output * op2);
	
	bool (*reduce_operand_pair)(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output ** op1, jive_output ** op2);
};

extern const jive_binary_operation_class JIVE_BINARY_OPERATION_;
#define JIVE_BINARY_OPERATION (JIVE_BINARY_OPERATION_.base)

/* node class inheritable methods */

jive_node_normal_form *
jive_binary_operation_get_default_normal_form_(const jive_node_class * cls, jive_node_normal_form * parent, struct jive_graph * graph);

bool
jive_binary_operation_can_reduce_operand_pair_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * op1, jive_output * op2);

bool
jive_binary_operation_reduce_operand_pair_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output ** op1, jive_output ** op2);

/* normal form class */

struct jive_binary_operation_normal_form_class {
	jive_node_normal_form_class base;
	void (*set_reducible)(jive_binary_operation_normal_form * self, bool enable);
	void (*set_flatten)(jive_binary_operation_normal_form * self, bool enable);
	void (*set_reorder)(jive_binary_operation_normal_form * self, bool enable);
	void (*set_distribute)(jive_binary_operation_normal_form * self, bool enable);
	void (*set_factorize)(jive_binary_operation_normal_form * self, bool enable);
	jive_output * (*normalized_create)(const jive_binary_operation_normal_form * self, struct jive_region * region, const jive_node_attrs * attrs, size_t noperands, jive_output * const operands[]);
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
			return (jive_binary_operation_normal_form *)cls;
		cls = cls->parent;
	}
	return 0;
}

jive_output *
jive_binary_operation_normalized_create(
	const jive_node_class * cls,
	struct jive_region * region,
	const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[]);

JIVE_EXPORTED_INLINE jive_output *
jive_binary_operation_normalized_create_new(
	const jive_binary_operation_normal_form * self,
	struct jive_region * region,
	const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[])
{
	const jive_binary_operation_normal_form_class * cls;
	cls = (const jive_binary_operation_normal_form_class *) self->base.class_;
	
	return cls->normalized_create(self, region, attrs, noperands, operands);
}


JIVE_EXPORTED_INLINE bool
jive_binary_operation_can_reduce_operand_pair(const jive_binary_operation_normal_form * self, const jive_node_attrs * attrs, jive_output * op1, jive_output * op2)
{
	return ((const jive_binary_operation_class *)self->base.node_class)->can_reduce_operand_pair(self->base.node_class, attrs, op1, op2);
}

JIVE_EXPORTED_INLINE bool
jive_binary_operation_reduce_operand_pair(const jive_binary_operation_normal_form * self, const jive_node_attrs * attrs, jive_output ** op1, jive_output ** op2)
{
	return ((const jive_binary_operation_class *)self->base.node_class)->reduce_operand_pair(self->base.node_class, attrs, op1, op2);
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
jive_binary_operation_operands_are_normalized_(const jive_node_normal_form * self, size_t noperands, jive_output * const operands[], const jive_node_attrs * attrs);

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

jive_output *
jive_binary_operation_normalized_create_(
	const jive_binary_operation_normal_form * self,
	struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[]);

#endif
