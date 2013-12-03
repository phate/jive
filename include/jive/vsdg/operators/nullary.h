/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_OPERATORS_NULLARY_H
#define JIVE_VSDG_OPERATORS_NULLARY_H

#include <jive/common.h>
#include <jive/vsdg/node.h>

struct jive_node;
struct jive_output;
struct jive_region;

typedef struct jive_node_class jive_nullary_operation_class;
typedef struct jive_nullary_operation_normal_form_class jive_nullary_operation_normal_form_class;
typedef struct jive_nullary_operation_normal_form jive_nullary_operation_normal_form;

/* node class */

extern const jive_node_class JIVE_NULLARY_OPERATION;

JIVE_EXPORTED_INLINE jive_output *
jive_nullary_operation_create_normalized(const jive_nullary_operation_class * cls,
	jive_graph * graph, const jive_node_attrs * attrs)
{
	jive_output * result;
	jive_node_create_normalized(cls, graph, attrs, 0, NULL, &result);
	return result;
}

/* node class inheritable methods */

jive_node_normal_form *
jive_nullary_operation_get_default_normal_form_(const jive_node_class * cls, jive_node_normal_form * parent, struct jive_graph * graph);

/* normal form class */

struct jive_nullary_operation_normal_form_class {
	jive_node_normal_form_class base;
	jive_output * (*normalized_create)(const jive_nullary_operation_normal_form * self, struct jive_region * region, const jive_node_attrs * attrs);
};

extern const jive_nullary_operation_normal_form_class JIVE_NULLARY_OPERATION_NORMAL_FORM_;
#define JIVE_NULLARY_OPERATION_NORMAL_FORM (JIVE_NULLARY_OPERATION_NORMAL_FORM_.base)

struct jive_nullary_operation_normal_form {
	jive_node_normal_form base;
};

JIVE_EXPORTED_INLINE jive_output *
jive_nullary_operation_normalized_create(
	const jive_nullary_operation_normal_form * self,
	struct jive_region * region,
	const jive_node_attrs * attrs)
{
	const jive_nullary_operation_normal_form_class * cls;
	cls = (const jive_nullary_operation_normal_form_class *) self->base.class_;
	
	return cls->normalized_create(self, region, attrs);
}

/* normal form inheritable methods */

bool
jive_nullary_operation_normalize_node_(const jive_node_normal_form * self, jive_node * node);

jive_output *
jive_nullary_operation_normalized_create_(const jive_nullary_operation_normal_form * self, struct jive_region * region, const jive_node_attrs * attrs);

#endif
