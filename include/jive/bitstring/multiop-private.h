#ifndef JIVE_BITSTRING_MULTIOP_PRIVATE_H
#define JIVE_BITSTRING_MULTIOP_PRIVATE_H

#include <jive/bitstring/multiop.h>
#include <jive/bitstring/type.h>

void
_jive_bitstring_multiop_node_init(
	jive_node * self,
	struct jive_region * region,
	size_t noperands,
	struct jive_output * operands[const],
	size_t nbits);

bool
_jive_bitstring_multiop_node_equiv(const jive_node_attrs * first, const jive_node_attrs * second);

bool
_jive_bitstring_multiop_node_can_reduce(const jive_output * first, const jive_output * second);

void
_jive_bitstring_keepwidth_multiop_node_init(
	jive_node * self,
	struct jive_region * region,
	size_t noperands,
	struct jive_output * operands[const]);

void
_jive_bitstring_expandwidth_multiop_node_init(
	jive_node * self,
	struct jive_region * region,
	size_t noperands,
	struct jive_output * operands[const]);

#endif
