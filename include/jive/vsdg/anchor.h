/*
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_ANCHOR_H
#define JIVE_VSDG_ANCHOR_H

#include <jive/vsdg/node.h>

/* anchor node */

typedef struct jive_anchor_node jive_anchor_node;
typedef struct jive_node_class jive_anchor_node_class;

extern const jive_anchor_node_class JIVE_ANCHOR_NODE;

struct jive_anchor_node {
	jive_node base;
};

JIVE_EXPORTED_INLINE struct jive_anchor_node *
jive_anchor_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ANCHOR_NODE))
		return (struct jive_anchor_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_anchor_node *
jive_anchor_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ANCHOR_NODE))
		return (const struct jive_anchor_node *) node;
	else
		return NULL;
}

/* node class inheritable methods */

jive_node_normal_form *
jive_anchor_node_get_default_normal_form_(const jive_node_class * cls,
	jive_node_normal_form * parent, struct jive_graph * graph);

/* normal form class */

typedef struct jive_anchor_node_normal_form jive_anchor_node_normal_form;
typedef struct jive_anchor_node_normal_form_class jive_anchor_node_normal_form_class;

struct jive_anchor_node_normal_form_class {
	jive_node_normal_form_class base;
	void (*set_reducible)(jive_anchor_node_normal_form * self, bool enable);
};

extern const jive_anchor_node_normal_form_class JIVE_ANCHOR_NODE_NORMAL_FORM_;
#define JIVE_ANCHOR_NODE_NORMAL_FORM (JIVE_ANCHOR_NODE_NORMAL_FORM_.base)

struct jive_anchor_node_normal_form {
	jive_node_normal_form base;
	bool enable_reducible;
};

JIVE_EXPORTED_INLINE jive_anchor_node_normal_form *
jive_anchor_node_normal_form_cast(jive_node_normal_form * self)
{
	if (jive_node_normal_form_isinstance(self, &JIVE_ANCHOR_NODE_NORMAL_FORM))
		return (jive_anchor_node_normal_form *) self;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE void
jive_anchor_node_normal_form_set_reducible(jive_anchor_node_normal_form * self, bool reducible)
{
	const jive_anchor_node_normal_form_class * cls;
	cls = (const jive_anchor_node_normal_form_class *) self->base.class_;
	cls->set_reducible(self, reducible);	
}

/* normal form class inhertiable methods */

void
jive_anchor_node_normal_form_init_(jive_anchor_node_normal_form * self,
	const jive_node_class * cls, jive_node_normal_form * parent_, struct jive_graph * graph);

void
jive_anchor_node_normal_form_set_reducible_(jive_anchor_node_normal_form * self_, bool reducible);

#endif 
