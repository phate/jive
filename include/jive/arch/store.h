/*
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_STORE_H
#define JIVE_ARCH_STORE_H

#include <jive/vsdg/node.h>

/* store normal form */

typedef struct jive_store_node_normal_form jive_store_node_normal_form;
typedef struct jive_store_node_normal_form_class jive_store_node_normal_form_class;

struct jive_store_node_normal_form_class {
	jive_node_normal_form_class base;
	void (*set_reducible)(jive_store_node_normal_form * self, bool enable);
	void (*normalized_create)(const jive_store_node_normal_form * self,
		struct jive_region * region, const jive_node_attrs * attrs, jive_output * address,
		jive_output * value, size_t nstates, jive_output * const istates[], jive_output * ostates[]);
};

extern const jive_store_node_normal_form_class JIVE_STORE_NODE_NORMAL_FORM_;
#define JIVE_STORE_NODE_NORMAL_FORM (JIVE_STORE_NODE_NORMAL_FORM_.base)

struct jive_store_node_normal_form {
	jive_node_normal_form base;
	bool enable_reducible;
};

JIVE_EXPORTED_INLINE jive_store_node_normal_form *
jive_store_node_normal_form_cast(jive_node_normal_form * self)
{
	if (jive_node_normal_form_isinstance(self, &JIVE_STORE_NODE_NORMAL_FORM))
		return (jive_store_node_normal_form *) self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE void
jive_store_node_normalized_create(const jive_store_node_normal_form * self,
	struct jive_region * region, const jive_node_attrs * attrs, jive_output * address,
	jive_output * value, size_t nstates, jive_output * const istates[], jive_output * ostates[])
{
	const jive_store_node_normal_form_class * cls;
	cls = (const jive_store_node_normal_form_class *) self->base.class_;

	return cls->normalized_create(self, region, attrs, address, value, nstates, istates, ostates);
}

JIVE_EXPORTED_INLINE void
jive_store_node_set_reducible(jive_store_node_normal_form * self, bool reducible)
{
	const jive_store_node_normal_form_class * cls;
	cls = (const jive_store_node_normal_form_class *) self->base.class_;
	cls->set_reducible(self, reducible);
}

/* store node */

extern const jive_node_class JIVE_STORE_NODE;

typedef struct jive_store_node_attrs jive_store_node_attrs;
typedef struct jive_store_node jive_store_node;

struct jive_store_node_attrs {
	jive_node_attrs base;
	size_t nbits;
	struct jive_value_type * datatype;
};

struct jive_store_node {
	jive_node base;
	jive_store_node_attrs attrs;
};

struct jive_node *
jive_store_by_address_node_create(struct jive_region * region,
	struct jive_output * address,
	const struct jive_value_type * datatype, struct jive_output * value,
	size_t nstates, struct jive_output * const states[]);

void
jive_store_by_address_create(struct jive_output * address,
	const struct jive_value_type * datatype, struct jive_output * value,
	size_t nstates, struct jive_output * const states[], jive_output * ostates[]);

struct jive_node *
jive_store_by_bitstring_node_create(struct jive_region * region,
	struct jive_output * address, size_t nbits,
	const struct jive_value_type * datatype, struct jive_output * value,
	size_t nstates, struct jive_output * const states[]);

void
jive_store_by_bitstring_create(struct jive_output * address, size_t nbits,
	const struct jive_value_type * datatype, struct jive_output * value,
	size_t nstates, struct jive_output * const istates[], struct jive_output * ostates[]);

JIVE_EXPORTED_INLINE jive_store_node *
jive_store_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_STORE_NODE))
		return (jive_store_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_value_type *
jive_store_node_get_datatype(const jive_store_node * node)
{
	return node->attrs.datatype;
}

#endif
