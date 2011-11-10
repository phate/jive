#include <jive/bitstring/constant.h>

#include <string.h>

#include <jive/bitstring/type.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/operators.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

static void
_jive_bitconstant_node_init(
	jive_bitconstant_node * self,
	jive_region * region,
	size_t nbits, const char bits[]);

static void
_jive_bitconstant_node_fini(jive_node * self);

static char *
_jive_bitconstant_node_get_label(const jive_node * self);

static const jive_node_attrs *
_jive_bitconstant_node_get_attrs(const jive_node * self);

static bool
_jive_bitconstant_node_match_attrs(const jive_node * self, const jive_node_attrs * attrs);

static jive_node *
_jive_bitconstant_node_create(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_BITCONSTANT_NODE = {
	.parent = &JIVE_NULLARY_OPERATION,
	.fini = _jive_bitconstant_node_fini, /* override */
	.get_label = _jive_bitconstant_node_get_label, /* override */
	.get_attrs = _jive_bitconstant_node_get_attrs, /* override */
	.match_attrs = _jive_bitconstant_node_match_attrs, /* override */
	.create = _jive_bitconstant_node_create, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

static void
_jive_bitconstant_node_init(
	jive_bitconstant_node * self,
	jive_region * region,
	size_t nbits, const char bits[])
{
	JIVE_DECLARE_BITSTRING_TYPE(type, nbits);
	jive_node_init_(&self->base, region,
		0, NULL, NULL,
		1, &type);
	self->attrs.nbits = nbits;
	self->attrs.bits = jive_context_malloc(region->graph->context, nbits);
	size_t n;
	for(n=0; n<nbits; n++) self->attrs.bits[n] = bits[n];
}

static void
_jive_bitconstant_node_fini(jive_node * self_)
{
	jive_bitconstant_node * self = (jive_bitconstant_node *) self_;
	jive_context_free(self->base.graph->context, self->attrs.bits);
	jive_node_fini_(&self->base);
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

static bool
_jive_bitconstant_node_match_attrs(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_bitconstant_node_attrs * first = &((const jive_bitconstant_node *) self)->attrs;
	const jive_bitconstant_node_attrs * second = (const jive_bitconstant_node_attrs *) attrs;
	if (first->nbits != second->nbits) return false;
	size_t n;
	for(n=0; n<first->nbits; n++)
		if (first->bits[n] != second->bits[n]) return false;
	return true;
}

static jive_node *
_jive_bitconstant_node_create(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	const jive_bitconstant_node_attrs * attrs = (const jive_bitconstant_node_attrs *) attrs_;
	
	jive_bitconstant_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->base.class_ = &JIVE_BITCONSTANT_NODE;
	_jive_bitconstant_node_init(node, region, attrs->nbits, attrs->bits);
	
	return &node->base;
}

jive_node *
jive_bitconstant_create(jive_graph * graph, size_t nbits, const char bits[])
{
	jive_bitconstant_node_attrs attrs;
	attrs.nbits = nbits;
	attrs.bits = (char *) bits;
	
	return jive_nullary_operation_normalized_create(&JIVE_BITCONSTANT_NODE, graph->root_region, &attrs.base)->node;
}

jive_output *
jive_bitconstant(jive_graph * graph, size_t nbits, const char bits[])
{
	jive_bitconstant_node_attrs attrs;
	attrs.nbits = nbits;
	attrs.bits = (char *) bits;
	
	return jive_nullary_operation_normalized_create(&JIVE_BITCONSTANT_NODE, graph->root_region, &attrs.base);
}

jive_output *
jive_bitconstant_unsigned(struct jive_graph * graph, size_t nbits, uint64_t value)
{
	char bits[nbits];
	
	size_t i;
	for (i = 0; i < nbits; i++) {
		bits[i] = '0' + (value & 1);
		value = value >> 1;
	}
	
	jive_bitconstant_node_attrs attrs;
	attrs.nbits = nbits;
	attrs.bits = bits;

	return jive_nullary_operation_normalized_create(&JIVE_BITCONSTANT_NODE, graph->root_region,
		&attrs.base);	
}

jive_node *
jive_bitconstant_create_unsigned(struct jive_graph * graph, size_t nbits, uint64_t value)
{
	return jive_bitconstant_unsigned(graph, nbits, value)->node;
}

jive_output *
jive_bitconstant_signed(struct jive_graph * graph, size_t nbits, int64_t value)
{
	char bits[nbits];
	
	size_t i;
	for (i = 0; i < nbits; i++) {
		bits[i] = '0' + (value & 1);
		value = value >> 1;
	}
	
	jive_bitconstant_node_attrs attrs;
	attrs.nbits = nbits;
	attrs.bits = bits;

	return jive_nullary_operation_normalized_create(&JIVE_BITCONSTANT_NODE, graph->root_region,
		&attrs.base);	
}

jive_node *
jive_bitconstant_create_signed(struct jive_graph * graph, size_t nbits, int64_t value)
{
	return jive_bitconstant_signed(graph, nbits, value)->node;
}
