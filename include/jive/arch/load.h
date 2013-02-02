/*
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_LOAD_H
#define JIVE_ARCH_LOAD_H

#include <jive/vsdg/node.h>

/* load normal form */

typedef struct jive_load_node_normal_form jive_load_node_normal_form;
typedef struct jive_load_node_normal_form_class jive_load_node_normal_form_class;

struct jive_load_node_normal_form_class {
	jive_node_normal_form_class base;
	void (*set_reducible)(jive_load_node_normal_form * self, bool enable);
	jive_output * (*normalized_create)(const jive_load_node_normal_form * self,
		struct jive_region * region, const jive_node_attrs * attrs, jive_output * address,
		size_t nstates, jive_output * const states[]);
};

extern const jive_load_node_normal_form_class JIVE_LOAD_NODE_NORMAL_FORM_;
#define JIVE_LOAD_NODE_NORMAL_FORM (JIVE_LOAD_NODE_NORMAL_FORM_.base)

struct jive_load_node_normal_form {
	jive_node_normal_form base;
	bool enable_reducible;
};

JIVE_EXPORTED_INLINE jive_load_node_normal_form *
jive_load_node_normal_form_cast(jive_node_normal_form * self)
{
	const jive_node_normal_form_class * cls = self->class_;
	while (cls) {
		if (cls == &JIVE_LOAD_NODE_NORMAL_FORM)
			return (jive_load_node_normal_form *) self;
		cls = cls->parent;
	}
	return NULL;
}

JIVE_EXPORTED_INLINE jive_output *
jive_load_node_normalized_create(const jive_load_node_normal_form * self,
	struct jive_region * region, const jive_node_attrs * attrs, jive_output * address,
	size_t nstates, jive_output * const states[])
{
	const jive_load_node_normal_form_class * cls;
	cls = (const jive_load_node_normal_form_class *) self->base.class_;

	return cls->normalized_create(self, region, attrs, address, nstates, states);
}

JIVE_EXPORTED_INLINE void
jive_load_node_set_reducible(jive_load_node_normal_form * self, bool reducible)
{
	const jive_load_node_normal_form_class * cls;
	cls = (const jive_load_node_normal_form_class *) self->base.class_;
	cls->set_reducible(self, reducible);
}

/* load node */

extern const jive_node_class JIVE_LOAD_NODE;

typedef struct jive_load_node_attrs jive_load_node_attrs;
typedef struct jive_load_node jive_load_node;

struct jive_load_node_attrs {
	jive_node_attrs base;
	size_t nbits;
	struct jive_value_type * datatype;
};

struct jive_load_node {
	jive_node base;
	jive_load_node_attrs attrs;
};

struct jive_node *
jive_load_by_address_node_create(struct jive_region * region,
	struct jive_output * address,
	const struct jive_value_type * datatype,
	size_t nstates, struct jive_output * const states[]);

struct jive_output *
jive_load_by_address_create(struct jive_output * address,
	const struct jive_value_type * datatype,
	size_t nstates, struct jive_output * const states[]);

struct jive_node *
jive_load_by_bitstring_node_create(struct jive_region * region,
	struct jive_output * address, size_t nbits,
	const struct jive_value_type * datatype,
	size_t nstates, struct jive_output * const states[]);

struct jive_output *
jive_load_by_bitstring_create(struct jive_output * address,
	size_t nbits, const struct jive_value_type * datatype,
	size_t nstates, struct jive_output * const states[]);

JIVE_EXPORTED_INLINE jive_load_node *
jive_load_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_LOAD_NODE))
		return (jive_load_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_value_type *
jive_load_node_get_datatype(const jive_load_node * node)
{
	return node->attrs.datatype;
}

JIVE_EXPORTED_INLINE size_t
jive_load_node_get_nbits(const jive_load_node * node)
{
	return node->attrs.nbits;
}

#endif
