#ifndef JIVE_VSDG_FUNCTION_H
#define JIVE_VSDG_FUNCTION_H

#include <jive/common.h>

#include <jive/vsdg/functiontype.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>

extern const jive_node_class JIVE_APPLY_NODE;

typedef struct jive_node jive_apply_node;

jive_node *
jive_apply_node_create(jive_region * region, jive_output * function,
	size_t narguments, jive_output * const arguments[]);

JIVE_EXPORTED_INLINE jive_apply_node *
jive_apply_node_cast(jive_node * node)
{
	if(node->class_ == &JIVE_APPLY_NODE) return (jive_apply_node *) node;
	else return 0;
}

extern const jive_node_class JIVE_SYMBOLICFUNCTION_NODE;

typedef struct jive_symbolicfunction_node jive_symbolicfunction_node;
typedef struct jive_symbolicfunction_node_attrs jive_symbolicfunction_node_attrs;

struct jive_symbolicfunction_node_attrs {
	jive_node_attrs base;
	const char * name;
	jive_function_type type;
};

struct jive_symbolicfunction_node {
	jive_node base;
	jive_symbolicfunction_node_attrs attrs; 
};

jive_node *
jive_symbolicfunction_node_create(struct jive_graph * graph, const char * name, const jive_function_type * type);

jive_output *
jive_symbolicfunction_create(struct jive_graph * graph, const char * name, const jive_function_type * type);

JIVE_EXPORTED_INLINE jive_symbolicfunction_node *
jive_symbolicfunction_node_cast(jive_node * node)
{
	if(node->class_ == &JIVE_SYMBOLICFUNCTION_NODE) return (jive_symbolicfunction_node *) node;
	else return 0;
}

extern const jive_node_class JIVE_ENTER_NODE;
extern const jive_node_class JIVE_LEAVE_NODE;

jive_region *
jive_function_region_create(jive_region * parent);

extern const jive_node_class JIVE_LAMBDA_NODE;

typedef struct jive_lambda_node jive_lambda_node;
typedef struct jive_lambda_node_attrs jive_lambda_node_attrs;

struct jive_lambda_node_attrs {
	jive_node_attrs base;
	jive_function_type function_type;
	jive_gate ** argument_gates;
	jive_gate ** return_gates;
};

struct jive_lambda_node {
	jive_node base;
	jive_lambda_node_attrs attrs;
};

jive_node *
jive_lambda_node_create(jive_region * function_region);

jive_output *
jive_lambda_create(jive_region * function_region);

JIVE_EXPORTED_INLINE jive_lambda_node *
jive_lambda_node_cast(jive_node * node)
{
	if(node->class_ == &JIVE_LAMBDA_NODE) return (jive_lambda_node *) node;
	else return 0;
}

JIVE_EXPORTED_INLINE jive_node *
jive_lambda_node_get_enter_node(const jive_lambda_node * self)
{
	return self->base.inputs[0]->origin->node->region->top;
}

JIVE_EXPORTED_INLINE jive_node *
jive_lambda_node_get_leave_node(const jive_lambda_node * self)
{
	return self->base.inputs[0]->origin->node;
}

void
jive_inline_lambda_apply(jive_node * apply_node);

#endif
