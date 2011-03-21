#ifndef JIVE_BITSTRING_CONSTANT_H
#define JIVE_BITSTRING_CONSTANT_H

#include <jive/bitstring/type.h>
#include <jive/vsdg/node.h>

extern const jive_node_class JIVE_BITCONSTANT_NODE;

typedef struct jive_bitconstant_node jive_bitconstant_node;
typedef struct jive_bitconstant_node_attrs jive_bitconstant_node_attrs;

struct jive_bitconstant_node_attrs {
	jive_node_attrs base;
	size_t nbits;
	char * bits;
};

struct jive_bitconstant_node {
	jive_node base;
	jive_bitconstant_node_attrs attrs;
};

/**
	\brief Create bitconstant
	\param graph Graph to create constant in
	\param nbits Number of bits
	\param bits Values of bits
	\returns Bitstring value representing constant
	
	Create new bitconstant node.
*/
jive_node *
jive_bitconstant_create(struct jive_graph * graph, size_t nbits, const char bits[]);

/**
	\brief Create bitconstant
	\param graph Graph to create constant in
	\param nbits Number of bits
	\param bits Values of bits
	\returns Bitstring value representing constant
	
	Convenience function that either creates a new constant or
	returns the output handle of an existing constant.
*/
jive_output *
jive_bitconstant(struct jive_graph * graph, size_t nbits, const char bits[]);

static inline jive_bitconstant_node *
jive_bitconstant_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITCONSTANT_NODE) return (jive_bitconstant_node *) node;
	else return 0;
}

static inline bool
jive_bitconstant_is_zero(const jive_bitconstant_node * node)
{
	size_t n;
	for(n=0; n<node->attrs.nbits; n++) if (node->attrs.bits[n] != '0') return false;
	return true;
}

static inline bool
jive_bitconstant_is_one(const jive_bitconstant_node * node)
{
	size_t n;
	for(n=1; n<node->attrs.nbits; n++) if (node->attrs.bits[n] != '0') return false;
	return node->attrs.bits[0] == '1';
}

static inline bool
jive_bitconstant_is_minus_one(const jive_bitconstant_node * node)
{
	size_t n;
	for(n=0; n<node->attrs.nbits; n++) if (node->attrs.bits[n] != '1') return false;
	return true;
}

#endif
