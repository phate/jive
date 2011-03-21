#include <jive/vsdg/function.h>

#include <stdio.h>
#include <string.h>

#include <jive/common.h>
#include <jive/context.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

static void
_jive_apply_node_init(jive_apply_node * self, struct jive_region * region, jive_output * function,
	size_t narguments, jive_output * const arguments[]);

static jive_node *
_jive_apply_node_create(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

static char *
_jive_apply_node_get_label(const jive_node * self_);

const jive_node_class JIVE_APPLY_NODE = {
	.parent = &JIVE_NODE,
	.fini = _jive_node_fini, /* inherit */
	.get_label = _jive_apply_node_get_label, /* override */
	.get_attrs = _jive_node_get_attrs, /* inherit */
	.match_attrs = _jive_node_match_attrs, /* inherit */
	.create = _jive_apply_node_create, /* override */
	.get_aux_rescls = _jive_node_get_aux_rescls /* inherit */
};

static char *
_jive_apply_node_get_label(const jive_node * self_)
{
	return strdup("APPLY");
}

static jive_node *
_jive_apply_node_create(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[])
{
	return jive_apply_node_create(region, operands[0], noperands - 1, &operands[1]);
}

static void
_jive_apply_node_init(
	jive_apply_node * self,
	struct jive_region * region,
	jive_output * function,
	size_t narguments,
	jive_output * const arguments[])
{
	//FIXME: this is ugly, some other sort of error would be nice
	JIVE_DEBUG_ASSERT(function->class_ == &JIVE_FUNCTION_OUTPUT);
	jive_function_output * fct = (jive_function_output *) function;

	//FIXME: this is ugly, some other sort of error would be nice
	JIVE_DEBUG_ASSERT(fct->type.narguments == narguments);

	jive_output * args[narguments+1];
	const jive_type * argument_types[narguments+1];
	args[0] = function;
	argument_types[0] = &fct->type.base.base; 
 
	size_t i;
	for(i = 0; i < fct->type.narguments; i++){
		argument_types[i+1] = fct->type.argument_types[i];
		args[i+1] = arguments[i];
	}
	
	_jive_node_init(self, region,
		narguments + 1, argument_types, args,
		fct->type.nreturns, (const jive_type * const *) fct->type.return_types);
}

jive_node *
jive_apply_node_create(struct jive_region * region, jive_output * function,
	size_t narguments, jive_output * const arguments[])
{
	jive_apply_node * node = jive_context_malloc(region->graph->context, sizeof( * node));

	node->class_ = &JIVE_APPLY_NODE;
	_jive_apply_node_init(node, region, function, narguments, arguments);

	return node; 
}

static void
_jive_symbolicfunction_node_fini(jive_node * self_);

static char *
_jive_symbolicfunction_node_get_label(const jive_node * self_);

static const jive_node_attrs *
_jive_symbolicfunction_node_get_attrs(const jive_node * self);

static bool
_jive_symbolicfunction_node_match_attrs(const jive_node * self, const jive_node_attrs * attrs);

static jive_node *
_jive_symbolicfunction_node_create(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_SYMBOLICFUNCTION_NODE = {
	.parent = &JIVE_NODE,
	.fini = _jive_symbolicfunction_node_fini, /* override */
	.get_label = _jive_symbolicfunction_node_get_label, /* override */
	.get_attrs = _jive_symbolicfunction_node_get_attrs, /* inherit */
	.match_attrs = _jive_symbolicfunction_node_match_attrs, /* override */
	.create = _jive_symbolicfunction_node_create, /* override */
	.get_aux_rescls = _jive_node_get_aux_rescls /* inherit */
};

static void
_jive_symbolicfunction_node_init(
	jive_symbolicfunction_node * node,
	jive_graph * graph,
	const char * fctname,
	const jive_function_type * type)
{
	node->attrs.name = jive_context_strdup(graph->context, fctname);
	jive_function_type_init(&node->attrs.type, graph->context,
		type->narguments, (const jive_type **) type->argument_types,
		type->nreturns, (const jive_type **) type->return_types);

	const jive_type * rtype = &type->base.base;
	_jive_node_init(&node->base, graph->root_region,
		0, NULL, NULL,
		1, &rtype);
}

static void
_jive_symbolicfunction_node_fini(jive_node * self_)
{
	jive_symbolicfunction_node * self = (jive_symbolicfunction_node *) self_;
	
	jive_context_free(self_->graph->context, (char *)self->attrs.name);
	
	jive_function_type_fini(&self->attrs.type);
	
	_jive_node_fini(&self->base);
}

static char *
_jive_symbolicfunction_node_get_label(const jive_node * self_)
{
	const jive_symbolicfunction_node * self = (const jive_symbolicfunction_node *) self_;
	
	return strdup(self->attrs.name);
}

static jive_node *
_jive_symbolicfunction_node_create(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	const jive_symbolicfunction_node_attrs * attrs = (const jive_symbolicfunction_node_attrs *) attrs_;
	return jive_symbolicfunction_node_create(region->graph, attrs->name, &attrs->type);
}

static const jive_node_attrs *
_jive_symbolicfunction_node_get_attrs(const jive_node * self_)
{
	const jive_symbolicfunction_node * self = (const jive_symbolicfunction_node *) self_;
	
	return &self->attrs.base;
}

static bool
_jive_symbolicfunction_node_match_attrs(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_symbolicfunction_node_attrs * first = &((const jive_symbolicfunction_node *)self)->attrs;
	const jive_symbolicfunction_node_attrs * second = (const jive_symbolicfunction_node_attrs *) attrs;

	if (!jive_type_equals(&first->type.base.base, &second->type.base.base)) return false;
	if (strcmp(first->name, second->name)) return false;

	return true;
}

jive_node *
jive_symbolicfunction_node_create(struct jive_graph * graph, const char * name, const jive_function_type * type) 
{
	jive_symbolicfunction_node * node = jive_context_malloc(graph->context, sizeof(* node));
	node->base.class_ = &JIVE_SYMBOLICFUNCTION_NODE;
	_jive_symbolicfunction_node_init(node, graph, name, type);
	return &node->base;
} 

jive_output *
jive_symbolicfunction_create(struct jive_graph * graph, const char * name, const jive_function_type * type)
{
	return jive_symbolicfunction_node_create(graph, name, type)->outputs[0];
}
