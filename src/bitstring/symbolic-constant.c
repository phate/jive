#include <jive/bitstring/symbolic-constant.h>

#include <string.h>

#include <jive/bitstring/type.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/operators.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

static void
_jive_bitsymbolicconstant_node_init(
	jive_bitsymbolicconstant_node * self,
	jive_region * region,
	size_t nbits, const char bits[]);

static void
_jive_bitsymbolicconstant_node_fini(jive_node * self);

static char *
_jive_bitsymbolicconstant_node_get_label(const jive_node * self);

static const jive_node_attrs *
_jive_bitsymbolicconstant_node_get_attrs(const jive_node * self);

static bool
_jive_bitsymbolicconstant_node_match_attrs(const jive_node * self, const jive_node_attrs * attrs);

static jive_node *
_jive_bitsymbolicconstant_node_create(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_BITSYMBOLICCONSTANT_NODE = {
	.parent = &JIVE_NULLARY_OPERATION,
	.fini = _jive_bitsymbolicconstant_node_fini, /* override */
	.get_label = _jive_bitsymbolicconstant_node_get_label, /* override */
	.get_attrs = _jive_bitsymbolicconstant_node_get_attrs, /* override */
	.match_attrs = _jive_bitsymbolicconstant_node_match_attrs, /* override */
	.create = _jive_bitsymbolicconstant_node_create, /* override */
	.get_aux_rescls = _jive_node_get_aux_rescls /* inherit */
};

static void
_jive_bitsymbolicconstant_node_init(
	jive_bitsymbolicconstant_node * self,
	jive_region * region,
	size_t nbits, const char * name)
{
	JIVE_DECLARE_BITSTRING_TYPE(type, nbits);
	_jive_node_init(&self->base, region,
		0, NULL, NULL,
		1, &type);
	self->attrs.nbits = nbits;
	size_t len = strlen(name);
	self->attrs.name = jive_context_malloc(region->graph->context, len + 1);
	size_t n;
	for(n=0; n<len+1; n++) self->attrs.name[n] = name[n];
}

static void
_jive_bitsymbolicconstant_node_fini(jive_node * self_)
{
	jive_bitsymbolicconstant_node * self = (jive_bitsymbolicconstant_node *) self_;
	jive_context_free(self->base.graph->context, self->attrs.name);
	_jive_node_fini(&self->base);
}

static char *
_jive_bitsymbolicconstant_node_get_label(const jive_node * self_)
{
	const jive_bitsymbolicconstant_node * self = (const jive_bitsymbolicconstant_node *) self_;
	
	return strdup(self->attrs.name);
}

static const jive_node_attrs *
_jive_bitsymbolicconstant_node_get_attrs(const jive_node * self_)
{
	const jive_bitsymbolicconstant_node * self = (const jive_bitsymbolicconstant_node *) self_;
	return &self->attrs.base;
}

static bool
_jive_bitsymbolicconstant_node_match_attrs(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_bitsymbolicconstant_node_attrs * first = &((const jive_bitsymbolicconstant_node *)self)->attrs;
	const jive_bitsymbolicconstant_node_attrs * second = (const jive_bitsymbolicconstant_node_attrs *) attrs;
	if (first->nbits != second->nbits) return false;
	return strcmp(first->name, second->name) == 0;
}

static jive_node *
_jive_bitsymbolicconstant_node_create(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	const jive_bitsymbolicconstant_node_attrs * attrs = (const jive_bitsymbolicconstant_node_attrs *) attrs_;
	jive_bitsymbolicconstant_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->base.class_ = &JIVE_BITSYMBOLICCONSTANT_NODE;
	_jive_bitsymbolicconstant_node_init(node, region, attrs->nbits, attrs->name);
	return &node->base;
}

jive_node *
jive_bitsymbolicconstant_create(jive_graph * graph, size_t nbits, const char * name)
{
	jive_bitsymbolicconstant_node_attrs attrs;
	attrs.nbits = nbits;
	attrs.name = (char *)name;
	return jive_nullary_operation_normalized_create(&JIVE_BITSYMBOLICCONSTANT_NODE, graph->root_region, &attrs.base);
}

jive_output *
jive_bitsymbolicconstant(jive_graph * graph, size_t nbits, const char * name)
{
	jive_bitsymbolicconstant_node_attrs attrs;
	attrs.nbits = nbits;
	attrs.name = (char *)name;
	return jive_nullary_operation_normalized_create(&JIVE_BITSYMBOLICCONSTANT_NODE, graph->root_region, &attrs.base)->outputs[0];
}
