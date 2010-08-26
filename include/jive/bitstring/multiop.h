#ifndef JIVE_BITSTRING_MULTIOP_H
#define JIVE_BITSTRING_MULTIOP_H

#include <jive/vsdg/node.h>
#include <jive/bitstring/type.h>

extern const jive_node_class JIVE_BITSTRING_MULTIOP_NODE;
extern const jive_node_class JIVE_BITSTRING_KEEPWIDTH_MULTIOP_NODE;
extern const jive_node_class JIVE_BITSTRING_EXPANDWIDTH_MULTIOP_NODE;

typedef struct jive_node jive_bitand_node;
extern const jive_node_class JIVE_BITAND_NODE;
jive_bitand_node *
jive_bitand_node_create(
	struct jive_region * region,
	size_t noperands, struct jive_output * operands[const]);

typedef struct jive_node jive_bitor_node;
extern const jive_node_class JIVE_BITOR_NODE;
jive_bitor_node *
jive_bitor_node_create(
	struct jive_region * region,
	size_t noperands, struct jive_output * operands[const]);

typedef struct jive_node jive_bitxor_node;
extern const jive_node_class JIVE_BITXOR_NODE;
jive_bitxor_node *
jive_bitxor_node_create(
	struct jive_region * region,
	size_t noperands, struct jive_output * operands[const]);

typedef struct jive_node jive_bitsum_node;
extern const jive_node_class JIVE_BITSUM_NODE;
jive_bitsum_node *
jive_bitsum_node_create(
	struct jive_region * region,
	size_t noperands, struct jive_output * operands[const]);

typedef struct jive_node jive_bitproduct_node;
extern const jive_node_class JIVE_BITPRODUCT_NODE;
jive_bitproduct_node *
jive_bitproduct_node_create(
	struct jive_region * region,
	size_t noperands, struct jive_output * operands[const]);

typedef struct jive_node jive_bitconcat_node;
extern const jive_node_class JIVE_BITCONCAT_NODE;
jive_bitconcat_node *
jive_bitconcat_node_create(
	struct jive_region * region,
	size_t noperands, struct jive_output * operands[const]);

#endif
