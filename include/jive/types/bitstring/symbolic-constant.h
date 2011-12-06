#ifndef JIVE_TYPES_BITSTRING_SYMBOLIC_CONSTANT_H
#define JIVE_TYPES_BITSTRING_SYMBOLIC_CONSTANT_H

#include <jive/vsdg/node.h>
#include <jive/types/bitstring/type.h>

extern const jive_node_class JIVE_BITSYMBOLICCONSTANT_NODE;

typedef struct jive_bitsymbolicconstant_node jive_bitsymbolicconstant_node;
typedef struct jive_bitsymbolicconstant_node_attrs jive_bitsymbolicconstant_node_attrs;

struct jive_bitsymbolicconstant_node_attrs {
	jive_node_attrs base;
	size_t nbits;
	char * name;
};

struct jive_bitsymbolicconstant_node {
	jive_node base;
	jive_bitsymbolicconstant_node_attrs attrs;
};

/**
	\brief Create symbolic constant
	\param graph Graph to create constant in
	\param nbits Number of bits
	\param name Symbol name
	\returns Bitstring value representing constant
	
	Create new bitconstant node.
*/
jive_node *
jive_bitsymbolicconstant_create(struct jive_graph * graph, size_t nbits, const char * name);

/**
	\brief Create symbolic constant
	\param graph Graph to create constant in
	\param nbits Number of bits
	\param name Symbol name
	\returns Bitstring value representing constant
	
	Convenience function that either creates a new constant or
	returns the output handle of an existing constant.
*/
jive_output *
jive_bitsymbolicconstant(struct jive_graph * graph, size_t nbits, const char * name);

JIVE_EXPORTED_INLINE jive_bitsymbolicconstant_node *
jive_bitsymbolicconstant_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITSYMBOLICCONSTANT_NODE) return (jive_bitsymbolicconstant_node *) node;
	else return 0;
}

#endif
