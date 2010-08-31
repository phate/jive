#include <jive/bitstring/constant.h>
#include <jive/bitstring/type.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg.h>
#include <string.h>
#include <jive/vsdg/normalization-private.h>

static void
_jive_bitconstant_node_init(
	jive_bitconstant_node * self,
	jive_graph * graph,
	size_t nbits, const char bits[]);

static void
_jive_bitconstant_node_fini(jive_node * self);

static char *
_jive_bitconstant_node_get_label(const jive_node * self);

static const jive_node_attrs *
_jive_bitconstant_node_get_attrs(const jive_node * self);

static jive_node *
_jive_bitconstant_node_create(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * operands[]);

static bool
_jive_bitconstant_node_equiv(const jive_node_attrs * first, const jive_node_attrs * second);

const jive_node_class JIVE_BITCONSTANT_NODE = {
	.parent = &JIVE_NODE,
	.fini = _jive_bitconstant_node_fini, /* override */
	.get_label = _jive_bitconstant_node_get_label, /* override */
	.get_attrs = _jive_bitconstant_node_get_attrs, /* override */
	.create = _jive_bitconstant_node_create, /* override */
	.equiv = _jive_bitconstant_node_equiv, /* override */
	.can_reduce = _jive_node_can_reduce, /* inherit */
	.reduce = _jive_node_reduce, /* inherit */
	.get_aux_regcls = _jive_node_get_aux_regcls /* inherit */
};

static void
_jive_bitconstant_node_init(
	jive_bitconstant_node * self,
	jive_graph * graph,
	size_t nbits, const char bits[])
{
	JIVE_DECLARE_BITSTRING_TYPE(type, nbits);
	_jive_node_init(&self->base, graph->root_region,
		0, NULL, NULL,
		1, &type);
	self->attrs.nbits = nbits;
	self->attrs.bits = jive_context_malloc(graph->context, nbits);
	size_t n;
	for(n=0; n<nbits; n++) self->attrs.bits[n] = bits[n];
}

static void
_jive_bitconstant_node_fini(jive_node * self_)
{
	jive_bitconstant_node * self = (jive_bitconstant_node *) self_;
	jive_context_free(self->base.graph->context, self->attrs.bits);
	_jive_node_fini(&self->base);
}

static char *
_jive_bitconstant_node_get_label(const jive_node * self_)
{
	const jive_bitconstant_node * self = (const jive_bitconstant_node *) self_;
	
	char tmp[self->attrs.nbits + 1];
	size_t n;
	for(n=0; n<self->attrs.nbits; n++)
		tmp[n] = self->attrs.bits[self->attrs.nbits - n - 1];
	tmp[n] = 0;
	return strdup(tmp);
}

static const jive_node_attrs *
_jive_bitconstant_node_get_attrs(const jive_node * self_)
{
	const jive_bitconstant_node * self = (const jive_bitconstant_node *) self_;
	return &self->attrs.base;
}

static jive_node *
_jive_bitconstant_node_create(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * operands[])
{
	const jive_bitconstant_node_attrs * attrs = (const jive_bitconstant_node_attrs *) attrs_;
	return &jive_bitconstant_node_create(region->graph, attrs->nbits, attrs->bits)->base;
}

static bool
_jive_bitconstant_node_equiv(const jive_node_attrs * first_, const jive_node_attrs * second_)
{
	const jive_bitconstant_node_attrs * first = (const jive_bitconstant_node_attrs *) first_;
	const jive_bitconstant_node_attrs * second = (const jive_bitconstant_node_attrs *) second_;
	if (first->nbits != second->nbits) return false;
	size_t n;
	for(n=0; n<first->nbits; n++)
		if (first->bits[n] != second->bits[n]) return false;
	return true;
}

jive_bitconstant_node *
jive_bitconstant_node_create(jive_graph * graph, size_t nbits, const char bits[])
{
	jive_bitconstant_node * node = jive_context_malloc(graph->context, sizeof(*node));
	node->base.class_ = &JIVE_BITCONSTANT_NODE;
	_jive_bitconstant_node_init(node, graph, nbits, bits);
	return node;
}

jive_bitstring *
jive_bitconstant_create(jive_graph * graph, size_t nbits, const char bits[])
{
	jive_bitconstant_node_attrs attrs;
	attrs.nbits = nbits;
	attrs.bits = (char *) bits;
	jive_node * node = jive_node_cse(&JIVE_BITCONSTANT_NODE, graph, &attrs.base, 0, NULL);
	if (!node) node = (jive_node *) jive_bitconstant_node_create(graph, nbits, bits);
	return (jive_bitstring_output *) node->outputs[0];
}
