#ifndef JIVE_BITSTRING_CONSTANT_H
#define JIVE_BITSTRING_CONSTANT_H

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

jive_bitconstant_node *
jive_bitconstant_node_create(struct jive_graph * graph, size_t nbits, const char bits[]);

#endif
