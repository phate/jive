#ifndef JIVE_VSDG_FUNCTION_H
#define JIVE_VSDG_FUNCTION_H

#include <jive/vsdg/functiontype.h>
#include <jive/vsdg/node.h>

extern const jive_node_class JIVE_APPLY_NODE;

typedef struct jive_node jive_apply_node;

jive_node *
jive_apply_node_create(struct jive_region * region, jive_output * function,
	size_t narguments, jive_output * const arguments[]);

static inline jive_apply_node *
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

static inline jive_symbolicfunction_node *
jive_symbolicfunction_node_cast(jive_node * node)
{
	if(node->class_ == &JIVE_SYMBOLICFUNCTION_NODE) return (jive_symbolicfunction_node *) node;
	else return 0;
}

#endif
